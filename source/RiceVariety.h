#pragma once

#include "stdafx.h"
#include <ctime>

class Date
{
public:
    int                 day;
    int                 month;
    int                 year;
    string              actualDate;

    Date(string &str, int daysAagePeeche = 0) : actualDate(str)
    {
        year = 2014; // unused so marking as random

        size_t dayEnd = str.find_first_of('/');
        if (dayEnd != string::npos)
        {
            day = atoi((str.substr(0, dayEnd)).c_str());
        }
        else
        {
            throw "ERROR: input date is '" + str + "' is not in correct format. Use dd/mm format ";
        }

        //size_t monthEnd = str.find_first_of('/', dayEnd + 1);
        //if (monthEnd != string::npos)
        //{
        month = atoi((str.substr(dayEnd+1)).c_str());
        //}
        //else
        //{
        //    throw "ERROR: input date is '" + str + "' is not in correct format. Use dd/mm format ";
        //}

        if (day < 1 || day > 31 || month < 1 || month > 12)
        {
            throw "ERROR: input date is '" + str + "' is not in correct range" ;
        }

        struct tm dateStructObj = {0, 0, 12};
        dateStructObj.tm_year = year - 1900;
        dateStructObj.tm_mon = month - 1;
        dateStructObj.tm_mday = day;

        //DatePlusDays(&dateStructObj, daysAagePeeche);

        year = dateStructObj.tm_year + 1900;
        month = dateStructObj.tm_mon + 1;
        day = dateStructObj.tm_mday;
    }

    // Adjust date by a number of days +/-
    void DatePlusDays( struct tm* indate, int days )
    {
        const time_t ONE_DAY = 24 * 60 * 60 ;

        // Seconds since start of epoch
        time_t date_seconds = mktime( indate ) + (days * ONE_DAY) ;

        // Update caller's date
        // Use localtime because mktime converts to UTC so may change date
        *indate = *localtime( &date_seconds );
    }

    bool operator<=(const Date &rhs)
    {
        if (this->month == rhs.month)
        {
            if (this->day <= rhs.day)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (this->month < rhs.month)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool operator<(const Date &rhs)
    {
        if (this->month == rhs.month)
        {
            if (this->day < rhs.day)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else if (this->month < rhs.month)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool operator==(const Date &rhs)
    {
        if (this->month == rhs.month)
        {
            if (this->day == rhs.day)
            {
                return true;
            }
        }
        return false;
    }
    //friend ofstream& operator<<(ofstream &output, const Date &d);
};

//ofstream& operator<<(ofstream &output, const Date &d)
//{
//    output << d.day << "/" << d.month;
//    return output;
//}

class riceVariety
{
public:
    string              name;
    Date                sDate;
    Date                eDate;
    set<riceVariety *>  nonConflictingNeighbours;
    set<riceVariety *>  conflictingNeighbours;
    bool                used;
    bool                blocked;
    bool                dumped;

    riceVariety(string &varname, string &sd, string &ed) : name(varname), used(false), sDate(sd, 0), eDate(ed, 0), blocked(false), dumped(false)
    {}

    ~riceVariety()
    {}

    bool pollinationConflicts(riceVariety *rhs)
    {
        const time_t THREE_DAY = 3 * 24 * 60 * 60;

        struct tm dateStructObjThis = {0, 0, 12};
        dateStructObjThis.tm_year = (this->sDate.year) - 1900;
        dateStructObjThis.tm_mon = (this->sDate.month) - 1;
        dateStructObjThis.tm_mday = (this->sDate.day);

        // Seconds since start of epoch
        time_t date_seconds_this = mktime( &dateStructObjThis );

        struct tm dateStructObjRhs = {0, 0, 12};
        dateStructObjRhs.tm_year = (rhs->sDate.year) - 1900;
        dateStructObjRhs.tm_mon = (rhs->sDate.month) - 1;
        dateStructObjRhs.tm_mday = (rhs->sDate.day);

        time_t date_seconds_rhs = mktime(&dateStructObjRhs);

        if (date_seconds_rhs > date_seconds_this)
        {
            if (date_seconds_rhs - date_seconds_this < THREE_DAY)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else 
        {
            if (date_seconds_this - date_seconds_rhs < THREE_DAY)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
};