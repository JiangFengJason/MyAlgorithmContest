using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OSprocess_resource_management
{
    class Program
    {
        static void Main(string[] args)
        {
            string inputPath;
            string outputPath;
            string line;
            string[] lines;
            PRM prm=new PRM();
            prm.initPRM();
            inputPath = "D:/gitfile/OSprocess_resource_management/input.txt";
            outputPath = "D:/gitfile/OSprocess_resource_management/output.txt";
            FileStream fs = new FileStream(outputPath, FileMode.Create, FileAccess.Write);
            fs.Close();
            StreamReader sr = new StreamReader(inputPath, Encoding.Default);
            StreamWriter sw = new StreamWriter(outputPath, true, Encoding.Default);
            //按行读取文件
            while(sr.Peek()>-1)
            {
                line = sr.ReadLine();
                lines = line.Split(' ');
                if (lines[0] == "init")
                {
                    prm.restore();
                    Console.WriteLine(prm.pcb[prm.current_running].pid);
                    sw.WriteLine(prm.pcb[prm.current_running].pid);
                    sw.Flush();
                }
                else if (lines[0] == "quit")
                {
                    break;
                }
                else if (lines[0] == "cr")
                {
                    prm.create(lines[1], int.Parse(lines[2]));
                    Console.WriteLine(prm.pcb[prm.current_running].pid);
                    sw.WriteLine(prm.pcb[prm.current_running].pid);
                    sw.Flush();
                }
                else if (lines[0] == "de")
                {
                    int t = prm.contain(lines[1]);
                    prm.destroy(t);
					Console.WriteLine(prm.pcb[prm.current_running].pid);
                    sw.WriteLine(prm.pcb[prm.current_running].pid);
                    sw.Flush();
                }
                else if (lines[0] == "req")
                {
                    if (lines[1] == "R1"&&int.Parse(lines[2])==1)
                    {
                        prm.request(0, int.Parse(lines[2]));
					    Console.WriteLine(prm.pcb[prm.current_running].pid);
                        sw.WriteLine(prm.pcb[prm.current_running].pid);
                        sw.Flush();
                    }
                    else if (lines[1] == "R2" && (0 < int.Parse(lines[2]) && int.Parse(lines[2]) <= 2))
                    {
                        prm.request(1, int.Parse(lines[2]));
                        Console.WriteLine(prm.pcb[prm.current_running].pid);
                        sw.WriteLine(prm.pcb[prm.current_running].pid);
                        sw.Flush();
                    }
                    else if (lines[1] == "R3" && (0 < int.Parse(lines[2]) && int.Parse(lines[2]) <= 3))
                    {
                        prm.request(2, int.Parse(lines[2]));
                        Console.WriteLine(prm.pcb[prm.current_running].pid );
                        sw.WriteLine(prm.pcb[prm.current_running].pid);
                        sw.Flush();
                    }
                    else if (lines[1] == "R4" && (0 < int.Parse(lines[2]) && int.Parse(lines[2]) <= 4))
                    {
                        prm.request(3, int.Parse(lines[2]));
                        Console.WriteLine(prm.pcb[prm.current_running].pid);
                        sw.WriteLine(prm.pcb[prm.current_running].pid);
                        sw.Flush();
                    }
                }
                else if (lines[0] == "rel")
                {
                    if (lines[1] == "R1" && int.Parse(lines[2]) == 1)
                    {
                        if (prm.pcb[prm.current_running].other_resource[0].used >= int.Parse(lines[2]))
                        {
                            prm.release(0, int.Parse(lines[2]));
                            Console.WriteLine(prm.pcb[prm.current_running].pid );
                            sw.WriteLine(prm.pcb[prm.current_running].pid);
                            sw.Flush();
					    }
                    }
                    else if (lines[1] == "R2" && (0 < int.Parse(lines[2]) && int.Parse(lines[2]) <= 2))
                    {
                        if (prm.pcb[prm.current_running].other_resource[1].used >= int.Parse(lines[2]))
                        {
                            prm.release(1, int.Parse(lines[2]));
                            Console.WriteLine(prm.pcb[prm.current_running].pid );
                            sw.WriteLine(prm.pcb[prm.current_running].pid);
                            sw.Flush();
                        }
                    }
                    else if (lines[1] == "R3" && (0 < int.Parse(lines[2]) && int.Parse(lines[2]) <= 3))
                    {
                        if (prm.pcb[prm.current_running].other_resource[2].used >= int.Parse(lines[2]))
                        {
                            prm.release(2, int.Parse(lines[2]));
                            Console.WriteLine(prm.pcb[prm.current_running].pid );
                            sw.WriteLine(prm.pcb[prm.current_running].pid);
                            sw.Flush();
                        }
                    }
                    else if (lines[1] == "R4" && (0 < int.Parse(lines[2]) && int.Parse(lines[2]) <= 4))
                    {
                        if (prm.pcb[prm.current_running].other_resource[3].used >= int.Parse(lines[2]))
                        {
                            prm.release(3, int.Parse(lines[2]));
                            Console.WriteLine(prm.pcb[prm.current_running].pid );
                            sw.WriteLine(prm.pcb[prm.current_running].pid);
                            sw.Flush();
                        }
                    }
                }
                else if (lines[0] == "to")
                {
                    prm.timeout();
				    Console.WriteLine( prm.pcb[prm.current_running].pid );
                    sw.WriteLine(prm.pcb[prm.current_running].pid);
                    sw.Flush();
                }
            }
            sw.Close();
        }
    }
}
