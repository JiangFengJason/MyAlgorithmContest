using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Difference
{
    class Solution_1
    {
        public bool CanConstruct(string ransomNote, string magazine)
        {
            char[] ransomNot = ransomNote.ToArray();
            char[] magazin = magazine.ToArray();
            if (ransomNot.Length > magazin.Length)
            {
                return false;
            }

            int[] a = new int[26];// 最多有26个字母
            int[] b = new int[26];

            for (int i = 0; i < ransomNot.Length; i++)
            {
                a[ransomNot[i] - 'a']++;// 进行字符个数的判断，如果已经存在了，就++（这里最重要）
            }

            for (int i = 0; i < magazin.Length; i++)
            {
                b[magazin[i] - 'a']++;
            }

            for (int i = 0; i < a.Length; i++)
            {
                if (a[i] > b[i])
                {// 判断第一个数组中的每一个是否有大于第二个数组的值
                    return false;
                }
            }
            return true;
        }
    }
}
