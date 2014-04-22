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

volatile static long taskList[MAX_TASKS][5]; // taskID, interval, lastRun, cycles, isAlive
//int runningTask = 0;
EthernetClient client;

const IPAddress ip(192,168,1,2);

EthernetServer server(340);

void setup()
{
  Serial.begin(115200);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  Ethernet.begin(mac, ip);
  server.begin();
  for(int i = 0; i < MAX_TASKS; i++)
  {
    taskList[i][0] = 0;
  }
  createTask(1, 0, 0);
  //createTask(2, 500, 0);
  //createTask(3, 10000, 0);
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

void runTask(int task)
{
  if(task == 1)
  {
    tcpConnection();
  }
  else if(task == 2)
  {
    blinkzOne();
  }
  else if(task == 3)
  {
    blinkzTwo();
  }
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
          Serial.println("running task " + taskList[i][TASK_ID]);
          runTask(taskList[i][TASK_ID]);
          taskList[i][LASTRUN] = millis();
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
    Serial.println("client present");
    static boolean hello = false;
    
    if(client.connected())
    {
      
      if(client.available())
      {
        if(!hello)
        {
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
            client.println("Created task");
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
    Serial.println("awaiting connection");
    client = server.available();
  }

}

void blinkzOne()
{
  static boolean run = false;
  
  if(!run)
  {
    digitalWrite(LED1, HIGH);
  }
  else
  {
    if(millis() > lastBlink1 + FREQ)
    {
      digitalWrite(LED1, LOW);
      lastBlink1 = millis();
      run ^= run;
    }
  }
}

void blinkzTwo()
{
  static boolean run = false;
  
  if(!run)
  {
    digitalWrite(LED2, HIGH);
  }
  else
  {
    if(millis() > lastBlink2 + FREQ)
    {
      digitalWrite(LED2, LOW);
      lastBlink2 = millis();
      run ^= run;
    }
  }
}
