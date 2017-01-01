using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    class Solution_6
    {
        public int IslandPerimeter(int[,] grid)
        {
            if (grid == null || grid.GetLength(0) == 0 || grid.GetLength(1) == 0) return 0;

            int count = 0;
            int duplicate = 0;
            for (int i = 0; i < grid.GetLength(0); i++)
            {
                for (int j = 0; j < grid.GetLength(1); j++)
                {
                    if (grid[i,j] == 1)
                    {
                        count += 4;

                        if (i != 0)
                        {
                            if (grid[i - 1,j] == 1)
                            {
                                duplicate++;
                            }
                        }
                        if (i != grid.GetLength(0) - 1)
                        {
                            if (grid[i + 1,j] == 1)
                            {
                                duplicate++;
                            }
                        }
                        if (j != 0)
                        {
                            if (grid[i,j-1] == 1)
                            {
                                duplicate++;
                            }
                        }
                        if (j != grid.GetLength(1) - 1)
                        {
                            if (grid[i,j+1] == 1)
                            {
                                duplicate++;
                            }
                        }

                    }

                }
            }
            return count - duplicate;  
        }
    }
}
