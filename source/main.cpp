#include "stdafx.h"
#include "Farm.h"

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        cout << "ERROR: incorrect number of arguments provided as input. Check README file for more help" << endl;
        return 1;
    }
    
    int rows = atoi(argv[1]);
    int columns = atoi(argv[2]);

    if (rows < 1 || columns < 1)
    {
        cout << "ERROR: incorrect arguments provided as input. Check README file for more help" << endl;
    }
    
    cout << "INFO: creating farm with rows = "<< rows << " columns = " << columns << endl;
    farm *riceFarm = new farm(rows, columns, argv[3]);
    if (argc > 4 && argv[4])
    {
        riceFarm->maxTimeSec = atoi(argv[4]);
    }
    if (argc > 5 && argv[5])
    {
        riceFarm->allowedRemoval = atoi(argv[5]);
    }
    riceFarm->placeRiceinGrid();

    delete riceFarm;
    char ch;
    cin >>  ch;
    return 0;
}
