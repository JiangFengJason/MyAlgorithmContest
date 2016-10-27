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
            //打印出一个字符list
            //IList<string> list = new Solution_1().FizzBuzz(15);
            //foreach(String s in list){
            //    Console.WriteLine(s);
            //}

            //打印出数组中唯一的一个数字
            int[] number = { 2,1,1 };
            int m = new Solution_2().SingleNumber(number);
            Console.WriteLine(m);
        }
        
        
    }
}
