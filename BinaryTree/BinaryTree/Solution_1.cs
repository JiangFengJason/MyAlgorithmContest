using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BinaryTree
{
    class Solution_1
    {
        public TreeNode InvertTree(TreeNode root)
        {
            if (root == null)
                return null;
            TreeNode ptmpNode = root.left;
            root.left = InvertTree(root.right);
            root.right = InvertTree(ptmpNode);
            return root;
        }
    }
}
