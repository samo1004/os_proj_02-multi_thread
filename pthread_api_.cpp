#include <unistd.h>    /* Symbolic Constants */
#include <sys/types.h> /* Primitive System Data Types */
#include <errno.h>     /* Errors */
#include <stdio.h>     /* Input/Output */
#include <stdlib.h>    /* General Utilities */
#include <pthread.h>   /* POSIX Threads */
#include <string.h>    /* String handling */
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <set>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <map>
#include <stdio.h>
#include <vector>
using namespace std;

void *avg_cos(void *ptr);

struct threadData
{
    pthread_t tid; // thread id
    string docid;  // document id
    string document;
    map<string, int> countmap; // counting how much words
    vector<int> term_freq;
    double cos_sim_coefficient;
};
threadData thdatas[50]; // max 50 cases
int c = 0;              // input cases counting
int main(int argc, char *argv[])
{
    clock_t mstart = clock(); // record the start time
    fstream f;
    string filename = argv[1]; // read in the file name
    f.open(filename, ios::in); // open the file in the folder

    string s;
    set<string> dict; // a set for recording non-repeated words
    stringstream ss2;
    while (getline(f, s)) // read in id and doc
    {
        for (int fna = 0; fna < s.length(); fna++)
        {
            if (ispunct(s[fna])) // if there is a punctuation in the string turn it into a blank
            {
                s[fna] = ' ';
                // cout<<"\n\na punct found\n\n";
            }
        }
        if (c % 2 == 0) // all the odd lines are document ids
        {
            // cout << s << endl;
            for (int di = 0; di < s.length(); di++)
            {
                if (!isdigit(s[di]))
                {
                    s.erase(di, 1);
                }
            }
            ss2 << s;
            ss2 >> thdatas[c / 2].docid;
            ss2.clear();
            // thdatas[c / 2].docid = s;
            //  thdatas[c / 2].docid.pop_back();
            //   cout << thdatas[c / 2].docid << endl;
        }
        else
        {
            thdatas[c / 2].document = s; // even lines are documents
        }
        c++;
    }
    c /= 2; // actual cases(2 lines an input document)
    // for (int i = 0; i < c; i++)
    // {
    //     cout << thdatas[i].docid;
    //     cout << " ";
    //     cout << thdatas[i].document << endl;
    // }
    f.close();
    for (int i = 0; i < c; i++) // term frequency counting for each document
    {
        stringstream ss(thdatas[i].document); // stringstream for spliting documents by blanks
        string word;
        while (true)
        {
            ss >> word;      // a word
            bool na = false; // not alpha
            if (ss.fail())
            {
                break;
            }
            if (word[word.length() - 1] == '\0') // avoid unknown error
            {
                word.resize(word.length() - 1);
            }
            thdatas[i].countmap[word]++; // count freq
            dict.insert(word);           // if there is a new word for set ,insert it
            // word.clear();
            for (int check = 0; check < word.size(); check++)
            {
                if (!isalpha(word[check]))
                {
                    na = true;
                }
            }
            if (na)
            {
                dict.erase(word);
                thdatas[i].countmap[word]--;
                // cout<<"deleted "<<word<<endl;
            }
        }
    }

    // for (auto &it : dict)
    // {
    //     cout << it << endl;
    // }

    for (int i = 0; i < c; i++) // creating the result term_frequency_chart in a vector
    {
        for (auto &t : dict)
        {
            thdatas[i].term_freq.push_back(thdatas[i].countmap[t]);
        }
    }
    // for (int i = 0; i < c; i++)
    // {
    //     cout << thdatas[i].docid << ":";
    //     for (auto &t : thdatas[i].term_freq)
    //     {
    //         cout << t << " ";
    //     }
    //     cout << endl;
    // }
    // clock_t midclk=clock();
    // cout<<double((midclk-mstart))/CLOCKS_PER_SEC*1000<<"ms\n";
    pthread_t pth[c];

    for (int i = 0; i < c; i++)
    {
        pthread_create(&pth[i], NULL, avg_cos, (void *)&thdatas[i]);
        cout << "[Main thread]:create TID:" << pth[i] << ",DocID:" << thdatas[i].docid << endl;
    }
    for (int i = 0; i < c; i++)
    {
        pthread_join(pth[i], NULL);
    }
    // for (int i = 0; i < c; i++)
    // {
    //     cout << thdatas[i].docid << ":";
    //     cout << thdatas[i].cos_sim_coefficient << endl;
    // }
    double highest = 0;
    int hid = 0;
    for (int i = 0; i < c; i++)
    {
        if (thdatas[i].cos_sim_coefficient >= highest)
        {
            if (thdatas[i].cos_sim_coefficient == highest)
            {
                if (thdatas[i].tid < thdatas[hid].tid)
                {
                    hid = i;
                    highest = thdatas[i].cos_sim_coefficient;
                }
            }
            else
            {
                hid = i;
                highest = thdatas[i].cos_sim_coefficient;
            }
        }
    }
    cout << "[Main thread] KeyDocID:" << thdatas[hid].docid << " Highest Average Cosine:" << setprecision(4) << fixed << highest << endl;
    clock_t mend = clock();
    cout << "[Main thread] CPU time: " << (double(mend - mstart)) / CLOCKS_PER_SEC * 1000 << "ms\n";
    return 0;
}

void *avg_cos(void *th)
{
    clock_t th_start, th_end;
    th_start = clock();
    threadData *thdata = (threadData *)th; // pass in a pointer to the threadData struct
    thdata->tid = pthread_self();
    cout << "[Tid=" << thdata->tid << "] DocID:" << thdata->docid << " [";
    for (int i = 0; i < thdata->term_freq.size() - 1; i++)
    {
        cout << thdata->term_freq[i] << ",";
    }
    cout << thdata->term_freq[thdata->term_freq.size() - 1] << "]" << endl;
    double c1, c2, c3;
    for (int i = 0; i < c; i++)
    {
        if (thdata->docid == thdatas[i].docid) // don't need to compare with itself
        {
            continue;
        }
        c1 = c2 = c3 = 0;
        for (int j = 0; j < thdata->term_freq.size(); j++)
        {
            c1 += thdata->term_freq[j] * thdatas[i].term_freq[j];
            c2 += thdata->term_freq[j] * thdata->term_freq[j];
            c3 += thdatas[i].term_freq[j] * thdatas[i].term_freq[j];
        }
        double plus = c1 / (sqrt(c2) * sqrt(c3)); // the cosine sim
        cout << "[TID=" << thdata->tid << "] "
             << "cosine(" << thdata->docid << "," << thdatas[i].docid << ")=" << plus << endl;
        thdata->cos_sim_coefficient += plus;
    }
    thdata->cos_sim_coefficient /= c - 1;
    th_end = clock();
    cout << "[Tid=" << thdata->tid << "] Avg_cosine: " << setprecision(4) << fixed << thdata->cos_sim_coefficient << endl;
    cout << "[Tid=" << thdata->tid << "] CPU time:" << (th_end - th_start) / (double)CLOCKS_PER_SEC * 1000 << "ms" << endl;
    pthread_exit(NULL);
}
