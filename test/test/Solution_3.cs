using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    class Solution_3
    {
        public int GetSum(int a, int b)
        {
            while (Convert.ToBoolean((a & b)<<1))
            {
                int temp = a;
                a = a ^ b;
                b = (temp & b) << 1;

            }
            return a ^ b;
        }
    }
}
