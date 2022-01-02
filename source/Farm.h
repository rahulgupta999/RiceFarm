#pragma once

#include "RiceVariety.h"

class plot
{
public:
    plot(int i=0, int j=0, int count =-1) : rowIndex(i), colIndex(j), plotNameAsIndex(count), assignedVariety(NULL), maxAssignedVariety(NULL) {}

    int                                 rowIndex;
    int                                 colIndex;
    int                                 plotNameAsIndex;
    riceVariety                        *assignedVariety;
    riceVariety                        *maxAssignedVariety;
};

class farm
{
public:
    farm(int i = 0, int j = 0, char *inputFilePath = NULL, char *outputFilePath = NULL);
    ~farm();

    void                                placeRiceinGrid();
    void                                copyRiceVarietyToMaxRiceVariety();
    int                                 maxTimeSec;
    int                                 allowedRemoval;
    int                                 pollinationDays;

private:
    vector<vector<plot *> >             farmPlotAdjacencyMap;
    plot                             ***farmGrid;
    vector<plot *>                      farmPlotList;
    vector<riceVariety *>               riceVarietyList;
    map<string, vector<riceVariety *> >   riceVarietyDateMap;
    int                                 rows;
    int                                 columns;
    int                                 maxRiceVarietyUsedCount;
    bool                                bestEffortSolution;
    string                              inputFilePath;
    string                              outputFilePath;
    clock_t                             beginTime;
    clock_t                             endTime;
    int                                 maxBlanks;
    int                                 totalRemoved;

    int                                 removeVariety();
    void                                resetFarm();
    void                                populateRiceVarietyList();
    void                                populateNonConflictingNeighbours();
    void                                populateConflictingNeighbours();
    void                                sortListAsPerDesiredOrder();
    bool                                recurseAndAssignVarietyToPlots(int currentPlotIndex, int remainingBlanks);
    bool                                willItConflict(int currentPlotIndex, riceVariety *possibleVariety);
    void                                decompileFarmGridInCSV();
    string                              getStringFromInt(int num);
};