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
            //int[] number = { 2,1,1 };
            //int m = new Solution_2().SingleNumber(number);
            //Console.WriteLine(m);

            //无加号，实现加法器
            //int n = new Solution_3().GetSum(1, 2);
            //Console.WriteLine(n);

            //把所有的0移动到数组的最后，并且不改变原来元素的相对位置
            //int [] num={0,1,0,3,12};
            //new Solution_4().MoveZeroes(num);

            //找出数组中的缺失元素
            //int[] num = { 4, 3, 2, 7, 8, 2, 3, 1 };
            //IList<int> b= new Solution_5().FindDisappearedNumbers(num);
            //foreach (int c in b)
            //{
            //    Console.WriteLine(c);
            //}

            int[,] grid = { { 0, 1, 0, 0 }, { 1, 1, 1, 0 }, { 0, 1, 0, 0 }, { 1, 1, 0, 0 } };
            int result = new Solution_6().IslandPerimeter(grid);
            Console.WriteLine(result);
        }
        
        
    }
}
