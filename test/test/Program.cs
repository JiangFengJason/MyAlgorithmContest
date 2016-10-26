using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    class Program
    {
        static void Main(string[] args)
        {
            IList<string> list = new Solution_1().FizzBuzz(15);
            foreach(String s in list){
                Console.WriteLine(s);
            }
        }
        
        
    }
}
