'''
MIT License

Copyright (c) 2021 Reese Grimsley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

'''

import numpy as np
import pandas as pd
from matplotlib import pyplot as plt

LATENCY_FILENAME_JAMMER = 'eth/L2_latency_jammer.log'
LATENCY_FILENAME_NO_JAMMER = 'eth/L2_latency_no_jammer.log'
LATENCY_FILE_COLUMNS = ['test_id', 'priority', 'frame_id', 'latency']

MAX_SAMPLES = 1000
LATENCY_LOWER_LIM_MICROSEC = 100

def read_latency_log(filename=LATENCY_FILENAME_JAMMER):
    '''
    Read the file containing latency logs as a pandas dataframe
    '''

    #I wish I actually understood regex. Thanks SO: https://stackoverflow.com/a/10341729/14824181
    separator = "\s?[, ]\s?" #error in the data where one column was missing a comma. Still works if the CSV is properly formatted, but will treat any whitespace as a valid separator
    all_data = pd.read_csv(filename, names=LATENCY_FILE_COLUMNS, sep=separator)

    test_ids = all_data['test_id'].unique()

    test_sets = []

    for ti in test_ids:
        ts = all_data.loc[all_data['test_id'] == ti]

        test_sets.append(ts)


    return test_sets


def histogram_of_latencies(latency_df, is_jammed):

    test_ids = latency_df['test_id'].unique()
    if len(test_ids) != 1:
        raise ValueError('Pandas array has more than one test_id in it')

    test_id = test_ids[0]

    priority = latency_df['priority'].unique()[0]


    print("Show histogram for test_id %d, priority %d" % (test_id, priority))

    latencies = pd.array(latency_df['latency'])
    weights = np.ones_like(latencies)/len(latencies) #equal weighting for each sample to show as probability mass function

    num_bins = 15
    plt.hist(latencies * 1e6, num_bins, weights=weights)

    plt.ylabel('Likelihood per Bin (of %d samples)' % len(latencies))
    plt.xlabel('Latency (us)')

    plt.gca().set_xlim(left=LATENCY_LOWER_LIM_MICROSEC) #never seen less than 120 us


    title = 'Latency Histogram for VLAN priority %d with %sJamming' % (priority, '' if is_jammed else 'No ' )
    plt.title(title)

    plt.show()

def plot_stats(results):
    '''
    Plot the statistics for the latencies, both with and without the jammer, for each VLAN priority

    We'll show latency with error bars (logy) and packet drop rate (probability, linear). 

    :param results: an array of 'results', where each entry is a tuple of the form: (test_id, priority, is_jamming, sample_size, mean, std, median, drop_rate, entire_raw_dataframe)
    '''

    #entries are tuples (test_id, priority, is_jamming, sample_size, mean, std, median, drop_rate, df)

    means_jam = [0 for i in range(8)]
    means_nojam = [0 for i in range(8)]
    stds_jam = [0 for i in range(8)]
    stds_nojam = [0 for i in range(8)]
    drop_rates_jam = [0 for i in range(8)]
    drop_rates_nojam = [0 for i in range(8)]
    num_samples_jam = ['' for i in range(8)]
    num_samples_nojam = ['' for i in range(8)]

    fig = plt.figure()
    pl = fig.add_subplot(1, 1, 1)
    ax1 = plt.gca()

    ax2 = ax1.twinx()
    for r in results:
        if r[2]: #jamming
            means_jam[r[1]] = r[4]
            stds_jam[r[1]] = r[5]
            num_samples_jam[r[1]] = 'n = ' +  str(r[3]) if r[3] < MAX_SAMPLES else ''

            # jammer_line = ax1.errorbar(r[1]+0.1, r[4], yerr=[ [min(r[5],r[4])], [r[5]]], fmt='ro', label='latency w/ jammer')
            drop_rates_jam[r[1]] = r[-2]+.005
            
        else:  #no jamming
            means_nojam[r[1]] = r[4]
            stds_nojam[r[1]] = r[5]
            num_samples_nojam[r[1]] =  'n = ' +  str(r[3]) if r[3] < MAX_SAMPLES else ''
            # nojammer_line = ax1.errorbar(r[1]-0.1, r[4], yerr=[ [min(r[5],r[4])], [r[5]]], fmt='b*', label='latency w/o jammer')
            drop_rates_nojam[r[1]] = r[-2]+.005

    lower_lim_jam = np.min([means_jam, stds_jam], axis = 0)
    print(lower_lim_jam)
    ax1.errorbar(np.asarray(range(8))+0.1, means_jam, yerr = [lower_lim_jam, stds_jam], fmt='ro', label='latency w/ jammer')
    ax1.errorbar(np.asarray(range(8))-0.1, means_nojam, yerr=stds_nojam, fmt='b*', label='latency w/o jammer')

    ax1.set_xlabel('VLAN Priority')
    ax1.legend(loc='upper right')
    ax1.set_ylabel('Latency (µs)')

    ax1.set_yscale('log', nonposy='clip')
    ax1.set_ylim([0, ax1.get_ylim()[1]])
    
    rects_nojam = ax2.bar(np.asarray(range(8))-0.3, drop_rates_nojam, width=0.2, color='b', label='drop-rate w/o jammer', )
    rects_jam = ax2.bar(np.asarray(range(8))+0.3, drop_rates_jam, width=0.2, color='r', label='drop-rate w/ jammer')

    # plt.bar_label(rects_nojam, labels=num_samples_nojam, padding=2)
    # plt.bar_label(rects_jam, labels=num_samples_jam, padding=4)

    ax2.set_ylabel('Frame Drop Rate')
    ax2.legend(loc='right')

    ax1.set_title('Network Performance over VLAN Priorities')

    plt.show()


def calc_statistics(df):
    '''
    Calculate an approximate packet drop rate, mean, median of latency, std devition
    '''

    test_ids = df['test_id'].unique()
    if len(test_ids) != 1:
        raise ValueError('Pandas array has more than one test_id in it')

    test_id = test_ids[0]

    priority = df['priority'].unique()[0]
    print('Calc statistics for test_id, priority (%d, %d) ' % (test_id, priority)) 

    latency_arr = pd.array(df['latency']) * 1e6
    frame_ids = pd.array(df['frame_id'])

    num_samples = len(frame_ids)

    #If we received all the frames we were planning to, then adjust by first so we can use the last frame to calculate # missed frames
    if len(frame_ids) == MAX_SAMPLES:
        frame_ids -= frame_ids[0]

    l_mean = np.mean(latency_arr)
    l_median = np.median(latency_arr)
    l_std = np.std(latency_arr)

    last_frame = np.max(frame_ids)
    capture_rate = len(frame_ids) / (last_frame + 1) # assume the sink was started before source, and source's frame id's start at 0
    drop_rate = 1 - capture_rate

    print(f'\tmean {l_mean}, \t std± {l_std} \n\tmedian {l_median} \n\tdrop_rate {drop_rate} \t(highest frame captured #{last_frame})')

    return num_samples, l_mean, l_std, l_median, drop_rate



if __name__ == "__main__":
    print("start")

    jammer_traces = read_latency_log(LATENCY_FILENAME_JAMMER)
    clean_traces = read_latency_log(LATENCY_FILENAME_NO_JAMMER)

    results = [] #entries are tuples (test_id, priority, is_jamming, sample_size, mean, std, median, drop_rate df)

    for jt in jammer_traces:
        test_id = jt['test_id'].unique()[0]
        priority = jt['priority'].unique()[0]

        sample_size, mean, std, median, drop_rate = calc_statistics(jt)
        results.append((test_id, priority, True, sample_size, mean, std, median, drop_rate, jt))
        histogram_of_latencies(jt, is_jammed=True)

    for ct in clean_traces:
        test_id = ct['test_id'].unique()[0]
        priority = ct['priority'].unique()[0]
        sample_size, mean, std, median, drop_rate = calc_statistics(ct)
        results.append((test_id, priority, False, sample_size, mean, std, median, drop_rate, ct))

        histogram_of_latencies(ct, is_jammed=False)

    print(results)
    print("Let's plot some stats")
    plot_stats(results)


    print("stop")