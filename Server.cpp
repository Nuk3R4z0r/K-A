#include <SPI.h>
#include <Ethernet.h>

String stringbuilder = "";
int lastBlink1 = 0;
int lastBlink2 = 0;

byte mac[] = { 0xFE, 0xED, 0xBE, 0xDE, 0xAD };

const int FREQ = 500;
const int MAX_TASKS = 10;
const int TASK_ID = 0;
const int INTERVAL = 1;
const int LASTRUN = 2;
const int CYCLES = 3;
const int IS_ALIVE = 4;
const int LED1 = 5;
const int LED2 = 6;
const int SPK = 7;

volatile static long taskList[MAX_TASKS][5]; // taskID, interval, lastRun, cycles, isAlive
//int runningTask = 0;
EthernetClient client;

const IPAddress ip(192,168,2,2);
const IPAddress gateway(192,168,2,1);
const IPAddress subnet(255, 255, 255, 0);

EthernetServer server(340);

void setup()
{
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(SPK, OUTPUT);
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  for(int i = 0; i < MAX_TASKS; i++)
  {
    taskList[i][0] = 0;
  }
  createTask(1, 0, 0);
  Serial.println("setup complete");
}

void createTask(int taskID, int interval, int cycles)
{
  for(int i = 0; i < MAX_TASKS; i++)
  {
    if(taskList[i][TASK_ID] == 0)
    {
      taskList[i][TASK_ID] = taskID;
      taskList[i][INTERVAL] = interval;
      taskList[i][LASTRUN] = 0;
      taskList[i][CYCLES] = cycles;
      taskList[i][IS_ALIVE] = 1;
      Serial.println("task created " + 1);
      break;
    }
  }
}

bool runTask(int task, int spot)
{
  if(task == 1)
  {
    tcpConnection();
    return true;
  }
  else if(task == 2)
  {
    blinkzOne();
    return true;
  }
  else if(task == 3)
  {
    blinkzTwo();
    return true;
  }
  else if(task == 4)
  {
    tone(SPK, taskList[spot][INTERVAL], taskList[spot][CYCLES]);
    return false;
  }

  return false;
}

void loop()
{
  for(int i = 0; i < MAX_TASKS; i++)
  {
    if(taskList[i][TASK_ID] != 0)
    {
      if(taskList[i][IS_ALIVE] == 1)
      {
        if(millis() > taskList[i][LASTRUN] + taskList[i][INTERVAL])
        {
          if(taskList[i][CYCLES] == 1)
          {
            taskList[i][IS_ALIVE] = 0;
          }

          //Serial.println("running task " + taskList[i][TASK_ID]);
          if(runTask(taskList[i][TASK_ID], i))
          {
            taskList[i][LASTRUN] = millis();
            taskList[i][CYCLES] -= 1;
          }
          else
          {
            taskList[i][IS_ALIVE] = 0;
          }
        }
      }
      else
      {
        taskList[i][TASK_ID] = 0;
        Serial.println("task deleted " + i);
      }
    }
  }
}

void tcpConnection()
{ 
  if(client)
  {
    static boolean hello = false;
    
    if(client.connected())
    {
      
      if(client.available())
      {
        if(!hello)
        {
          Serial.println("client present");
          client.println("Hello!");
          hello = true;
        }
        else
        {
          char s = client.read();
          
          if(!(s == 13 || s == 10))
          {
            stringbuilder += s;
          }
          
          if(s == '\n' && stringbuilder != "")
          {
            Serial.println(stringbuilder);
            Serial.println(Contains(stringbuilder, ","));
            if(Contains(stringbuilder, ","))
            {
              int id = stringbuilder.substring(IndexOf(stringbuilder, ",")).toInt();
              client.println(id);
              stringbuilder = RemoveFirst(stringbuilder, id + ",");
              client.println(stringbuilder);
              int cyc = stringbuilder.substring(IndexOf(stringbuilder, ",")).toInt();
              client.println(id);
              stringbuilder = RemoveFirst(stringbuilder, id + ",");
              client.println(stringbuilder);
              int inter = stringbuilder.substring(0).toInt();

              createTask(id, cyc, inter);
              client.print("Created task(");
              client.print(id);
              client.print(",");
              client.print(cyc);
              client.print(",");
              client.print(inter);
              client.println(")");
            }
            else
            {
              if(stringbuilder.toInt() >= 0)
              {
                tone(SPK, stringbuilder.toInt(), 200);
              }
            }

            stringbuilder = "";
          }
        }
      }
    }
    else
    {
      Serial.println("client disconnected");
      client.stop();
      hello = false;
    }
  }
  else
  {
    client = server.available();
  }

}

void blinkzOne()
{
  static boolean run = true;

  digitalWrite(LED1, run);
  
  run = !run;
}

void blinkzTwo()
{
  static boolean run = true;

  digitalWrite(LED2, run);
  
  run = !run;
}

bool Contains(String s, String search)
{
  int len = search.length();
  int max = s.length() - len; 

  for (int i=0; i<= max; i++) 
  {
    if (s.substring(i, len) == search)
    {
      return true;
    } 

  }

  return false;
}

int IndexOf(String s, String search)
{
  int len = search.length();
  int max = s.length() - len; 

  for (int i=0; i<= max; i++) 
  {
    if (s.substring(i, len) == search)
    {
      return i;
    } 

  }

  return -1;
}

String RemoveFirst(String s, String search)
{
  String newString = "";
  int len = search.length();
  int max = s.length() - len; 

  for (int i=0; i<= max; i++) 
  {
    if (s.substring(i, len) == search)
    {
      
      newString = s.substring(0, i) + s.substring(i + len);
      return newString;
    } 

  }

  return s;
}