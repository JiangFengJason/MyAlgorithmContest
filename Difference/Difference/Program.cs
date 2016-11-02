using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Difference
{
    class Program
    {
        static void Main(string[] args)
        {
            char a = new Solution().FindTheDifference("abcd", "abcde");
            Console.WriteLine(a);
        }
    }
}
