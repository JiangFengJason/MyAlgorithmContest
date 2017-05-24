using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace OSprocess_resource_management
{
    struct resource {
	    public int rid ;
        public int used ;
        public int wait_request ;
    };
    struct PCB {
        public string pid ;
        public string type ;
        public int id ;
        public int parent ;
        public int children ;
        public int younger ;
        public int older ;
        public int priority ;
        public resource[] other_resource;
    };
    struct RCB
    {
        public string rid ;
        public int initial ;
        public int remaining ;
        public List<int> wait_list;
    };
    class ProcessResourceManagement
    {
        
    }
    class PRM
    {

        private RCB R1;
        private RCB R2;
        private RCB R3;
        private RCB R4;
        private int delete_number = -1;
        List<int>[] rl=new List<int>[3];
        public int current_running;
        public RCB[] rcb = new RCB[4];
        public PCB [] pcb=new PCB[20];
        //函数集合：
        public void initPRM()
        {
            rl = new List<int>[3];
            for (int i = 0; i < 20; i++)
            {
                pcb[i].id = i;
            }
            pcb[0].pid = "init";
            pcb[0].priority = 0;
            //rl[0].Add(0);
            current_running = 0;

            R1.rid = "R1";
            R1.initial = 1;
            R1.remaining = 1;

            R2.rid = "R2";
            R2.initial = 2;
            R2.remaining = 2;

            R3.rid = "R3";
            R3.initial = 3;
            R3.remaining = 3;

            R4.rid = "R4";
            R4.initial = 4;
            R4.remaining = 4;

            rcb[0] = R1;
            rcb[1] = R2;
            rcb[2] = R3;
            rcb[3] = R4;

        }
        public void timeout()
        {
            int p = pcb[current_running].priority;
            rl[p].Remove(current_running);
            pcb[current_running].type = "ready";
            rl[p].Add(current_running);
            scheduler();
        }
        public void release(int n, int unit)
        {
            release2(n, unit);
            scheduler();
        }
        public void restore()
        {   
            for (int i = 1; i < 20; i++)
            {
                pcb[i].other_resource = new resource[4];
                pcb[i].pid = " ";
                pcb[i].type = "ready";
                pcb[i].parent = -1;
                pcb[i].children = -1;
                pcb[i].older = -1;
                pcb[i].younger = -1;
                pcb[i].priority = -1;
                pcb[i].other_resource[0].rid = -1;
                pcb[i].other_resource[1].rid = -1;
                pcb[i].other_resource[2].rid = -1;
                pcb[i].other_resource[3].rid = -1;
                pcb[i].other_resource[0].used = 0;
                pcb[i].other_resource[1].used = 0;
                pcb[i].other_resource[2].used = 0;
                pcb[i].other_resource[3].used = 0;
                pcb[i].other_resource[0].wait_request = 0;
                pcb[i].other_resource[1].wait_request = 0;
                pcb[i].other_resource[2].wait_request = 0;
                pcb[i].other_resource[3].wait_request = 0;
            }
            current_running = 0;

            for (int i = 0; i < 4; i++)
            {
                rcb[i].wait_list = new List<int>();
                rcb[i].initial = i + 1;
                rcb[i].remaining = i + 1;
                while (rcb[i].wait_list.Count>0)
                {
                    rcb[i].wait_list.RemoveAt(0);
                }
            }
            for (int i = 0; i < 3; i++)
            {
                rl[i] = new List<int>();
                while (rl[i].Count>0)
                {
                    rl[i].RemoveAt(rl[i].Count-1);
                }
            }
        }
        public void create(string name, int p) {
	        for (int i = 1; i < 20; i++) {
		        if (pcb[i].pid == " ") {
			        pcb[i].pid = name;
			        pcb[i].priority = p;
			        rl[p].Add(pcb[i].id);
			        pcb[i].parent = current_running;
			        for (int j = 1; j < 20; j++) {
				        if (j < i && pcb[j].parent == pcb[i].parent) {
					        pcb[j].younger = i;
					        pcb[i].older = j;
				        }
			        }
			        break;
		        }   
	        }   
	        scheduler();
        }
        public int scheduler()
        {
            //rl = new List<int>[3];
	        if (rl[2].Count>0) {
		        current_running = rl[2].First();
		        pcb[current_running].type = "running";
                return rl[2].First();
	        } else if (rl[1].Count>0) {
                current_running = rl[1].First();
		        pcb[current_running].type = "running";
                return rl[1].First();
	        } else {
		        current_running = 0;
		        pcb[current_running].type = "running";
		        return 0;
	        }
        }
        public void release2(int n, int unit)
        {
            pcb[current_running].other_resource[n].used -= unit;
            rcb[n].remaining += unit;
            int temp_pcb = rcb[n].wait_list.First();
            while (temp_pcb != 0 && pcb[temp_pcb].other_resource[n].wait_request <= rcb[n].remaining)
            {
                rcb[n].remaining -= pcb[temp_pcb].other_resource[n].wait_request;
                rcb[n].wait_list.Remove(temp_pcb);
                pcb[temp_pcb].type = "ready";
                pcb[temp_pcb].other_resource[n].used += pcb[temp_pcb].other_resource[n].wait_request;
                rl[pcb[temp_pcb].priority].Add(temp_pcb);
                temp_pcb = rcb[n].wait_list.FirstOrDefault();
            }
        }
        public void destroy(int n) {
	        for (int i = 0; i < 4; i++) {
		        if (pcb[n].other_resource[i].used != 0) {
			        release2(i ,pcb[n].other_resource[i].used);
			        pcb[n].other_resource[i].rid = -1;
			        pcb[n].other_resource[i].used = 0;
		        }
	        }
	        if (pcb[n].type == "ready" || pcb[n].type == "running") {
		        int p = pcb[n].priority;
		        rl[p].Remove(n);
	        }
	        else if ((pcb[n].type).CompareTo("blocked") == 0) {
		        for (int i = 0; i < 4; i++) {
			        rcb[i].wait_list.Remove(n);
		        }
	        }
	        for (int i = 0; i < 20; i++) {
		        if (pcb[i].parent == n) {
			        destroy(pcb[i].id);
		        }
		        if (pcb[i].id == n) {
			        pcb[i].pid = " ";
			        pcb[i].type = "ready";
			        pcb[i].parent = -1;
			        pcb[i].children = -1;
			        pcb[i].older = -1;
			        pcb[i].younger = -1;
			        pcb[i].priority = -1;
			        pcb[i].other_resource[0].rid = -1;
			        pcb[i].other_resource[1].rid = -1;
			        pcb[i].other_resource[2].rid = -1;
			        pcb[i].other_resource[3].rid = -1;
			        pcb[i].other_resource[0].used = 0;
			        pcb[i].other_resource[1].used = 0;
			        pcb[i].other_resource[2].used = 0;
			        pcb[i].other_resource[3].used = 0;
			        pcb[i].other_resource[0].wait_request = 0;
			        pcb[i].other_resource[1].wait_request = 0;
			        pcb[i].other_resource[2].wait_request = 0;
			        pcb[i].other_resource[3].wait_request = 0;
		        }
		        if (pcb[i].older == n) {
			        pcb[i].older = -1;
		        }
		        if (pcb[i].younger == n) {
			        pcb[i].younger = -1;
		        }
	        }
	        scheduler();
        }
        public int contain(string name) {
	        for (int i = 0; i < 20; i++) {
		        if (name.CompareTo(pcb[i].pid) == 0) {
			        return i;
		        }
	        }
	        return -1;
        }
        public void request(int n, int unit)
        {
            if (rcb[n].remaining >= unit)
            {
                rcb[n].remaining = rcb[n].remaining - unit;
                pcb[current_running].other_resource[n].rid = n;
                pcb[current_running].other_resource[n].used += unit;
            }
            else
            {
                pcb[current_running].type = "blocked";
                pcb[current_running].other_resource[n].wait_request += unit;
                rcb[n].wait_list.Add(current_running);
                rl[pcb[current_running].priority].Remove(current_running);
            }
            scheduler();
        }
    }
    class LN
    {
        
    }
    
}
