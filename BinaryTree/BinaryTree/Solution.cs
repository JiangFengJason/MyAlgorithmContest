using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BinaryTree
{
    class Solution
    {
        public int MaxDepth(TreeNode root) {
        int deep=0;
            if(root!=null){
               int lchilddeep=MaxDepth(root.left);
               int rchilddeep=MaxDepth(root.right);
               deep=lchilddeep>=rchilddeep?lchilddeep+1:rchilddeep+1;
            }
        return deep;
        }
    }
}
