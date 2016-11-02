using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Difference
{
    class Solution
    {
        public char FindTheDifference(string s, string t)
        {
            char[] ss = s.ToArray();
            char[] tt = t.ToArray();
            int[] count=new int [26];
            for (int i = 0; i < ss.Length; i++)
            {
                count[s[i] - 'a']++;
            }
            for (int j = 0; j < tt.Length; j++)
            {
                count[t[j] - 'a']--;
            }
            for (int i = 0; i < count.Length; i++)
            {
                if (count[i] == - 1)
                    return (char)(i + 'a');
            }
            return 'z';
        }
    }
}
