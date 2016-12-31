using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace test
{
    class Solution_5
    {
        public IList<int> FindDisappearedNumbers(int[] nums)
        {
            IList<int> ret= new List<int>();
            if(nums.Length==0)
                return ret;

            for(int i = 0; i < nums.Length; i ++){
                int val = Math.Abs(nums[i]) - 1;
                if(nums[val] > 0)
                  nums[val] = -nums[val];
            }

            for(int i = 0; i < nums.Length; i ++){
                if(nums[i] > 0)
                  ret.Add(i + 1);
            }
            return ret;
        }
    }
}
