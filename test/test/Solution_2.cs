using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    class Solution_2
    {
        public int SingleNumber(int[] nums)
        {
            int result = 0;
            for (int x = 0; x < nums.Length; ++x)
            {
                result = result ^ nums[x];
            }
            return result;
        }
    }
}
