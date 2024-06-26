#!/usr/bin/env python


##
# @dir
# In this directory, classes of the controller module are defined and implemented.


##
# In this file, Python functionality for automatic rating of Plexe result files on the mean deviation
# from the pre-defined gap is defined.
# To achieve this, the OMNeT++ Python API omnetpp.scave is used.
# Multithreading is introduced to speed up the processing of multiple evaluations.
# Wrapped by ConstantHeadway class.
#
# @file constant_headway.py
# @ingroup constant_headway

import os
import sys
from concurrent.futures import ThreadPoolExecutor

import numpy as np
import pandas as pd

sys.path.append(f'{os.environ["OMNETPP_HOME"]}/python')

# noinspection PyUnresolvedReferences
from omnetpp.scave import results as res, vectorops as ops


def get_last_value(df: pd.DataFrame) -> np.float128:
    ##
    # Returns the last value of the numpy array
    # located in the first row of the given DataFrame in field 'vecvalue'.
    #
    # @param df: A DataFrame containing a recorded vector of simulation data.
    # @return The longfloat at the last vector position in the first row of the DataFrame.
    vec = list(df.iloc[0]["vecvalue"])
    return np.float128(vec[len(vec) - 1])


def get_constant_headway(run_ids: list) -> np.float128:
    ##
    # Calculates a value rating the mean deviation of all vehicles from the pre-defined gap.
    # It calculates the mean squared deviation of each vehicle from its pre-defined gap,
    # adds that value up for each vehicle of a particular run and calculates the mean over all runs
    # (i.e., all repetitions and scenarios).
    #
    # @param run_ids: List of strings representing the OMNeT++ run ids of all runs to be evaluated.
    # @return A longfloat rating the deviation from the pre-defined gap.
    # @bug Running mean calculation over vectors using omnetpp.scave does not work correctly!

    # create filter for opp_scavetool to only use files with correct run ids
    run_filter = f"(run =~ {run_ids[0]}"
    for i in range(1, len(run_ids)):
        run_filter += f" OR run =~ {run_ids[i]}"
    run_filter += ")"

    # find out pre-defined gap
    name_filter = f"{run_filter} AND **.scenario.caccSpacing"
    headway = float(res.get_param_assignments(name_filter).iloc[0]["value"][0])

    # iterate over all simulation runs
    values = np.empty(len(run_ids), dtype=np.float128)
    for i in range(len(run_ids)):
        # get gap vectors of all but the first vehicles simulated in the given run
        name_filter = f"run =~ {run_ids[i]} AND distance AND module =~ **.node[*].appl AND NOT module =~ **.node[0]" \
                      f".appl"
        # fetch vectors
        vecs = res.get_vectors(name_filter)
        # calculate square error on each value
        vecs = ops.expression(vecs, f"(y - {headway}) ** 2")
        # calculate running mean over vectors - this statement is buggy for some reason
        vecs = ops.mean(vecs)
        # sum up the values of all vehicles
        vecs = ops.aggregate(vecs, "sum")
        # save the last value of the aggregated vector
        # (is equivalent with sum of means of all square errors since running mean was calculated)
        values.put(i, get_last_value(vecs))
    # return mean over all simulation runs
    return values.mean(dtype=np.float128)


def multithreaded(threads: int, directory: str, run_ids: list) -> list:
    ##
    # Runs get_constant_headway concurrently for multiple simulation
    # results with no more than the given number of threads.
    # This is the function actually called by ConstantHeadway.
    #
    # @param threads: Maximum number of threads to be used for concurrent execution.
    # @param directory: A path to the
    # directory directly or indirectly containing all result files that are to be evaluated.
    # @param run_ids: A list of lists of strings where each list of strings contains all OMNeT++ run ids of the runs
    # conducted for one parameterCombination
    # @return A list of longfloats representing the rating of the given simulation runs.

    res.set_inputs(directory)
    with ThreadPoolExecutor(max_workers=threads) as pool:
        results = pool.map(get_constant_headway, run_ids)
    return list(results)
