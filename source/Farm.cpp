// Ricefarm.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Farm.h"
#include <ctime>

struct comparator
{
    bool operator() (riceVariety *first, riceVariety *second)
    {
        return (first->nonConflictingNeighbours.size() < second->nonConflictingNeighbours.size());
    }
}comparatorObj;

farm::farm(int i, int j, char *inFilePath, char *outFilePath) : rows(i), columns(j), bestEffortSolution(false), maxTimeSec(300), totalRemoved(0)
{
    farmGrid = new plot**[rows];
    for (int index=0; index<rows; ++index)
    {
        farmGrid[index] = new plot*[columns];
    }

    int count = 1;
    for (int hindex=0; hindex<rows; ++hindex)
    {
        for (int vindex=0; vindex<columns; ++vindex)
        {
            plot *newplot = new plot(hindex, vindex, count);
            farmGrid[hindex][vindex] = newplot;
            farmPlotList.push_back(newplot);
            count++;
        }
    }

    farmPlotAdjacencyMap.resize(count);

    for (int hindex=0; hindex<rows; ++hindex)
    {
        for (int vindex=0; vindex<columns; ++vindex)
        {
            plot *plotObj = farmGrid[hindex][vindex];
            int row = plotObj->rowIndex;
            int col = plotObj->colIndex;

            vector<plot *> adjacenPlotList;
            if (row-1 >= 0 && col-1 >= 0)           adjacenPlotList.push_back(farmGrid[row-1][col-1]);
            if (row-1 >= 0)                         adjacenPlotList.push_back(farmGrid[row-1][col]);
            if (row-1 >= 0 && col+1 < columns)      adjacenPlotList.push_back(farmGrid[row-1][col+1]);
            if (col+1 < columns)                    adjacenPlotList.push_back(farmGrid[row][col+1]);
            if (row+1 < rows && col+1 < columns)    adjacenPlotList.push_back(farmGrid[row+1][col+1]);
            if (row+1 < rows)                       adjacenPlotList.push_back(farmGrid[row+1][col]);
            if (row+1 < rows && col-1 >= 0)         adjacenPlotList.push_back(farmGrid[row+1][col-1]);
            if (col-1 >= 0)                         adjacenPlotList.push_back(farmGrid[row][col-1]);

            farmPlotAdjacencyMap[plotObj->plotNameAsIndex - 1] = adjacenPlotList;
        }
    }

    maxRiceVarietyUsedCount = 0;

    if (inFilePath == NULL)
    {
        inFilePath = "input.csv";
    }
    inputFilePath = inFilePath;
    if (outFilePath == NULL)
    {
        outFilePath = "output.csv";
    }
    outputFilePath = inputFilePath + "_" + "output.csv";
}

farm::~farm()
{
    for (auto &obj : farmPlotList)
    {
        delete obj;
    }

    for (auto &variety: riceVarietyList)
    {
        delete variety;
    }

    for (int index=0; index<rows; ++index)
    {
        delete [] farmGrid[index];
    }
    delete [] farmGrid;
}

void farm::populateRiceVarietyList()
{
    ifstream infile;
    infile.open(inputFilePath.c_str());
    if (!infile)
    {
        throw "ERROR: Input file '" + inputFilePath + "' could not be opened, check if it is present";
    }

    string line;
    getline(infile, line);
    if(*line.rbegin() == '\r') { line.resize(line.size() - 1); }

    if (strcasecmp(line.c_str(), "Variety,Flowering Date") != 0)
    {
        throw (string("ERROR: input file CSV header '") + line + string("' is not as desired, check the sample"));
    }

    int lineNumber = 2;
    for (line.clear();getline(infile, line);line.clear())
    {
        if(*line.rbegin() == '\r') { line.resize(line.size() - 1); }
        string name, sd, ed;
        size_t nameEnd = line.find_first_of(',', 0);
        if (nameEnd != string::npos)
        {
            name = line.substr(0, nameEnd);
        }
        else
        {
            throw "ERROR: input file CSV is corrupt at line " + getStringFromInt(lineNumber)
        ;
        }

        /*size_t sdEnd = line.find_first_of(',', nameEnd + 1);
        if (sdEnd != string::npos)
        {
            sd = line.substr(nameEnd + 1, sdEnd - nameEnd - 1);
        }
        else
        {
            throw "ERROR: input file CSV is corrupt at line " + lineNumber;
        }*/

        //size_t edEnd = line.find_first_of('\0', sdEnd + 1);
        //if (edEnd != string::npos)
        //{
        //ed = line.substr(sdEnd + 1);
        sd = line.substr(nameEnd + 1);
        //}
        //else
        //{
        //    throw "ERROR: input file CSV is corrupt at line " + lineNumber;
        //}
        ++lineNumber;

        riceVariety *variety = new riceVariety(name, sd, sd);
        riceVarietyList.push_back(variety);
        (riceVarietyDateMap[variety->sDate.actualDate]).push_back(variety);
    }
    infile.close();
}

void farm::populateNonConflictingNeighbours()
{
    for (auto &variety : riceVarietyList)
    {
        (variety)->nonConflictingNeighbours.clear();
        (variety)->conflictingNeighbours.clear();
        if (variety->blocked) continue;
        for (auto &runner : riceVarietyList)
        {
            if (runner->blocked) continue;
            if (runner == variety)
            {
                continue;
            }
            else if (!variety->pollinationConflicts(runner))
            {
                (variety)->nonConflictingNeighbours.insert(runner);
            }
            else
            {
                (variety)->conflictingNeighbours.insert(runner);
            }
        }
    }
}

void farm::sortListAsPerDesiredOrder()
{
    //sort the map in decreasing order of conflict
    //std::sort(riceVarietyList.begin(), riceVarietyList.end(), comparatorObj);
}

void farm::resetFarm()
{
    for (auto & variety : riceVarietyList)
    {
        variety->used = false;
    }
    for (int hindex=0; hindex<rows; ++hindex)
    {
        for (int vindex=0; vindex<columns; ++vindex)
        {
            farmGrid[hindex][vindex]->assignedVariety = NULL;
        }
    }
    return;
}

int farm::removeVariety()
{
    map<string, vector<riceVariety *> >::iterator iter = riceVarietyDateMap.begin();
    vector<riceVariety *> maxVector;
    for (; iter != riceVarietyDateMap.end(); ++iter)
    {
        bool atLeastOneActive = false;
        for (auto &variety : (*iter).second)
        {
            if (!variety->blocked)
            {
                atLeastOneActive = true;
                break;
            }
        }
        if (atLeastOneActive)
        {
            if ((*iter).second.size() > maxVector.size())
            {
                maxVector = (*iter).second;
            }
        }
    }
    int count = floor(maxVector.size() / 2);
    int updated = 0;
    for (auto &variety : maxVector)
    {
        if (!count) break;
        if (!variety->blocked)
        {
            --count;
            ++updated;
            variety->blocked = true;
        }
    }
    return updated;
}

bool farm::recurseAndAssignVarietyToPlots(int currentPlotIndex, int remainingBlanks)
{
    if (currentPlotIndex >= (int)farmPlotList.size())
    {
        return true;
    }

    endTime = clock();
    double elapsed_secs = double(endTime - beginTime) / CLOCKS_PER_SEC;
    if (elapsed_secs > maxTimeSec)
    {
        if (allowedRemoval)
        {
            int noRemoved = removeVariety();
            maxBlanks += noRemoved;
            totalRemoved += noRemoved;
            if ((int)riceVarietyList.size() - totalRemoved < maxRiceVarietyUsedCount)
            {
                bestEffortSolution = true;
                return true;
            }
            --allowedRemoval;
            cout << "retrying after removing the variety" << endl;
            resetFarm();
            populateNonConflictingNeighbours();
            beginTime = clock();
            return recurseAndAssignVarietyToPlots(0, maxBlanks);
        }
        else
        {
            //cout << "could not assign all varieties in " << maxTimeSec << "seconds. Hence saving the best solution so far for used plot count: " << maxRiceVarietyUsedCount << endl;
            bestEffortSolution = true;
            return true;
        }
    }

    /*if (remainingBlanks > 0)
    {
        farmPlotList[currentPlotIndex]->assignedVariety = NULL;
        bool retValue = recurseAndAssignVarietyToPlots(currentPlotIndex + 1, --remainingBlanks);
        if (retValue == true)
        {
            return true;
        }
    }*/

    int allVarietyUsed = 0;
    for (int varietyIndex = 0; varietyIndex < (int)riceVarietyList.size(); ++varietyIndex)
    {
        riceVariety *currentVariety = riceVarietyList[varietyIndex];
        //cout << "CPI "<< currentPlotIndex << " checking rvi " << varietyIndex << endl;
        if (currentVariety->used == true || currentVariety->blocked == true)
        {
            ++allVarietyUsed;
          //  cout << "CPI "<< currentPlotIndex << "already used rvi " << varietyIndex << "  " << allVarietyUsed << endl;
            continue;
        }

        if (willItConflict(currentPlotIndex, currentVariety))
        {
          //  cout << "CPI "<< currentPlotIndex << "it will conflict rvi " << varietyIndex << endl;
            continue;
        }

        //cout << "CPI "<< currentPlotIndex << "assigning rvi " << varietyIndex << endl;
        farmPlotList[currentPlotIndex]->assignedVariety = currentVariety;
        currentVariety->used = true;

        //cout << "starting trails for plot index : " << currentPlotIndex + 1 << endl;
        bool retValue = recurseAndAssignVarietyToPlots(currentPlotIndex + 1, remainingBlanks);
        //cout << "CPI "<< currentPlotIndex << "after recursion got result " << retValue << " for rvi " << varietyIndex << endl;
        if (retValue == false)
        {
            farmPlotList[currentPlotIndex]->assignedVariety = NULL;
            currentVariety->used = false;
            continue;
        }
        else
        {
            return true;
        }
    }
    if (allVarietyUsed == riceVarietyList.size())
    {
        return true;
    }
    else if (remainingBlanks > 0)
    {
        farmPlotList[currentPlotIndex]->assignedVariety = NULL;
        bool retValue = recurseAndAssignVarietyToPlots(currentPlotIndex + 1, --remainingBlanks);
        if (retValue == true)
        {
            return true;
        }
    }
    
    if (allVarietyUsed > maxRiceVarietyUsedCount)
    {
        copyRiceVarietyToMaxRiceVariety();
        maxRiceVarietyUsedCount = allVarietyUsed;
    }
    return false;
}

bool farm::willItConflict(int currentPlotIndex, riceVariety *possibleVariety)
{
    //plot *currentPlot = farmPlotList[currentPlotIndex];
    for (auto &neighbouringPlot : farmPlotAdjacencyMap[currentPlotIndex])
    {
        riceVariety *varietyOnNeighbour = neighbouringPlot->assignedVariety;
        if (varietyOnNeighbour == NULL)
        {
            continue;
        }

        /*if ((possibleVariety->nonConflictingNeighbours).find(varietyOnNeighbour) == (possibleVariety->nonConflictingNeighbours).end())
        {
            return true;
        }*/

        if ((possibleVariety->conflictingNeighbours).find(varietyOnNeighbour) != (possibleVariety->conflictingNeighbours).end())
        {
            return true;
        }
    }
    return false;
}

string farm::getStringFromInt(int num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

void farm::placeRiceinGrid()
{
    try
    {
        cout << "INFO: populating rice variety list from input file : " << inputFilePath << endl;
        populateRiceVarietyList();

        int maxWhiteSpaces = 0;
        if (rows*columns < (int)riceVarietyList.size())
        {
            string errorStr("ERROR : Total number of plots ");
            char *temp = (char*)malloc(10);
            errorStr +=  getStringFromInt(farmPlotList.size()); // itoa((int)farmPlotList.size(), temp, 10);
            errorStr += " is less than the number of varieties ";
            errorStr += getStringFromInt(riceVarietyList.size());
            free(temp);
            throw errorStr;
        }
        else
        {
            maxWhiteSpaces = rows*columns - (int)riceVarietyList.size();
        }

        cout << "INFO: populating conflicting neighbours for rice variety size: " << riceVarietyList.size() << endl;
        populateNonConflictingNeighbours();

        //sortListAsPerDesiredOrder();

        beginTime = clock();
        maxRiceVarietyUsedCount = 0;
        cout << "INFO: try assigning varieties in the farm " << rows << "x" << columns << ". This may take some time " << endl;
        maxBlanks = maxWhiteSpaces;
        bool retValue = recurseAndAssignVarietyToPlots(0, maxWhiteSpaces);

        if (retValue == true)
        {
            cout << "INFO: decompiling output in file : " << outputFilePath << endl;
            decompileFarmGridInCSV();
        }
        else
        {
            throw "ERROR : could not find a possible solution for given list of variety and farm size,\n\tplease try by \n\tchanging the list of varieties OR \n\tincreasing farm dimensions";
        }
    }
    catch (string &errorStr)
    {
        cout << errorStr << endl;
    }
    catch (char *errorStr)
    {
        cout << errorStr << endl;
    }
    cout << "\n\tFINISHED !!!!" << endl;
}

void farm::decompileFarmGridInCSV()
{
    ofstream outfile;
    outfile.open(outputFilePath.c_str());
    if (!outfile)
    {
        throw "ERROR: Input file '" + outputFilePath + "' could not be opened, check if it is present";
    }

    for (int hindex=0; hindex<rows; ++hindex)
    {
        for (int vindex=0; vindex<columns; ++vindex)
        {
            riceVariety *variety = farmGrid[hindex][vindex]->assignedVariety;
            if (bestEffortSolution)
            {
                variety = farmGrid[hindex][vindex]->maxAssignedVariety;
            }
            if (variety)
            {
                variety->dumped = true;
                string vName = variety->name;
                Date dt = variety->sDate;
                outfile << vName.c_str() << "(FD : " << dt.day << "/" << dt.month << ") ,"; //" " << dt.day << "/" << dt.month << "," ;
            }
            else
            {
                outfile << " LEAVE EMPTY ,";
            }
        }
        outfile << endl;
    }
    outfile.close();

    for (auto &variety : riceVarietyList)
    {
        if (variety->dumped == false)
        {
            string vName = variety->name;
            Date dt = variety->sDate;
            cout << "Unused variety : " << vName.c_str() << "(FD : " << dt.day << "/" << dt.month << ") " << endl; //" " << dt.day << "/" << dt.month << "," ;
        }
    }

}

void farm::copyRiceVarietyToMaxRiceVariety()
{
    for (int hindex=0; hindex<rows; ++hindex)
    {
        for (int vindex=0; vindex<columns; ++vindex)
        {
            farmGrid[hindex][vindex]->maxAssignedVariety = farmGrid[hindex][vindex]->assignedVariety;
        }
    }
}