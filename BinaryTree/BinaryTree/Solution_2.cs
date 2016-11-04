using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BinaryTree
{
    class Solution_2
    {
        public int sumOfLeftLeaves(TreeNode root)
        {
            if (root==null) 
                return 0;
            if ((root.left!=null) && (root.left.left==null) && (root.left.right==null))
            {
                return root.left.val + sumOfLeftLeaves(root.right);
            }
            return sumOfLeftLeaves(root.left) + sumOfLeftLeaves(root.right);
        }
    }
}
