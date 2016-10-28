using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ReverseString
{
    class Program
    {
        static void Main(string[] args)
        {
            string sentence;
            sentence = System.Console.ReadLine();
            char[] chars = sentence.ToCharArray();
            Array.Reverse(chars);
            string result = new String(chars);
        }
    }
}
