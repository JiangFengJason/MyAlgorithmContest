using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Difference
{
    class Solution_2
    {
        public int[] Intersection(int[] nums1, int[] nums2)
        {
            List<int> m = new List<int>();
            int n1 = nums1.Length, n2 = nums2.Length;
            for (int i = 0; i < n1; i++)
            {
                if (!m.Contains(nums1[i]))
                    m.Add(nums1[i]);
            }
            List<int> a=new List<int>();
            for (int i = 0; i < n2; i++)
            {
                if (m.Contains(nums2[i]))
                {
                    a.Add(nums2[i]);
                    m.Remove(nums2[i]);
                }  
            }
            int[] res = a.ToArray();
            return res;
        }
    }
}
