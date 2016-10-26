using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    class Solution_1
    {
        public IList<string> FizzBuzz(int n)
        {
            IList<string> mlist = new List<string>();
            for (int i = 1; i <= n; ++i)
            {
                if (i % 3 == 0 && i % 5 != 0)
                {
                    mlist.Add("Fizz");
                }
                else if (i % 5 == 0 && i % 3 != 0)
                {
                    mlist.Add("Buzz");
                }
                else if (i % 3 == 0 && i % 5 == 0)
                {
                    mlist.Add("FizzBuzz");
                }
                else
                {
                    mlist.Add(i.ToString());
                }
            }
            return mlist;
        }
    }
}
