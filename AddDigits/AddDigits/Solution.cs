using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace AddDigits
{
    class Solution
    {
        public int AddDigits(int num)
        {
            while (num / 10 != 0)
            {
                num = num / 10 + num % 10;
            }
            return num;
        }
    }
}
