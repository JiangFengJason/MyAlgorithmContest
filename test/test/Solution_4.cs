using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    class Solution_4
    {
        public void MoveZeroes(int[] nums)
        {
            int i = 0;
            int j = 0;
            while (j < nums.Length)
            {
                if (nums[j] != 0)
                {
                    if (j != i)
                    {
                        nums[i++] = nums[j];
                        nums[j] = 0;
                    }
                    else
                    {
                        ++i;
                    }
                }
                ++j;
            }
        }
    }
}
