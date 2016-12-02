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
            //找出两个字符串中的不同的一个字母
            //char a = new Solution().FindTheDifference("abcd", "abcde");
            //Console.WriteLine(a);

//            bool is_can = new Solution_1().CanConstruct("fffbfg",
//"effjfggbffjdgbjjhhdegh");
//            Console.WriteLine(is_can);
            int[] nums1={1,2,2,1};
            int[] nums2={2,2};
            int[] result = new Solution_2().Intersection(nums1,nums2);
            foreach (int i in result)
            {
                Console.WriteLine(i);
            }
        }
    }
}
