#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string line, key;
  float totalMemory = 1, freeMemory = 1;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while(std::getline(stream, line))
    {
      std::istringstream linestream(line);
      linestream >> key;
      if(key == "MemTotal:")
        linestream >> totalMemory;
       else if(key == "MemAvailable:"){
         linestream >> freeMemory;
         break;
         }
    	
    
    }
    
  }
  
  return (totalMemory - freeMemory) / totalMemory; 
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() { 
  
  long uptime;
  string uptimestr, line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if(stream.is_open()){
    std::getline(stream, line);
  	std::istringstream linestream(line);
    
    linestream >> uptimestr;
  
  }
  
  uptime = std::stol(uptimestr);
  
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
   
  	return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies(); 
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) { 
 	string line, key;
  	vector<string> keys;
  
  	std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  	if (stream.is_open()){
    	
      std::getline(stream,line);
      std::istringstream linestream(line);
      while(linestream >> key){
      	keys.push_back(key);
      
      }
    
    }
    long utime = 0, stime = 0 , cutime = 0, cstime = 0;
  if (std::all_of(keys[13].begin(), keys[13].end(), isdigit))
    utime = stol(keys[13]);
  if (std::all_of(keys[14].begin(), keys[14].end(), isdigit))
    stime = stol(keys[14]);
  if (std::all_of(keys[15].begin(), keys[15].end(), isdigit))
    cutime = stol(keys[15]);
  if (std::all_of(keys[16].begin(), keys[16].end(), isdigit))
    cstime = stol(keys[16]);

  
  return (utime + stime + cutime + cstime) / sysconf(_SC_CLK_TCK);

}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  
  vector<string> jiffies = CpuUtilization();
  
  return stol(jiffies[CPUStates::kUser_]) + stol(jiffies[CPUStates::kNice_]) + stol(jiffies[CPUStates::kSystem_]) +
    	 stol(jiffies[CPUStates::kSoftIRQ_]) + stol(jiffies[CPUStates::kSystem_]) + stol(jiffies[CPUStates::kIRQ_]) + stol(jiffies[CPUStates::kSteal_]) ; 

}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  vector<string> jiffies = CpuUtilization();
  
  return stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]); 
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  string line, key, value;
  vector<string> ret;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()){
  	std::getline(stream,line);
    std::istringstream linestream(line);
    linestream >> key;
    
    while(linestream >> value){
 		ret.push_back(value);   
    }
    
  }
  return ret; 
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  int processes;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while(std::getline(stream, line))
    {
      std::istringstream linestream(line);
      linestream >> key;
      if(key == "processes"){

        linestream >> processes;
        break;
      		}    	
    
    	}
 	}
   return processes; 
}
// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  int runningProcesses;
  string line, key;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while(std::getline(stream, line))
    {
      std::istringstream linestream(line);
      linestream >> key;
      if(key == "procs_running"){

        linestream >> runningProcesses;
        break;
      }    	
    
    }
  
  }
  return runningProcesses;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string command;
  std::ifstream stream(kProcDirectory+std::to_string(pid)+kCmdlineFilename);
  if(stream.is_open()){
    std::getline(stream,command);
  }
  return command; 

}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line, key;
  int mem;
  std::ifstream stream(kProcDirectory+std::to_string(pid)+ kStatusFilename);
  if(stream.is_open()){
  	while(std::getline(stream,line)){
    	
      std::istringstream linestream(line);
      linestream >> key;
      if(key == "VmSize:"){
        linestream >> mem;
        mem /= 1000;
        break;
      
      }
    
    }
  
  }
  
  return std::to_string(mem); 
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) { 
  string line, key, uid;
  
  std::ifstream stream(kProcDirectory+std::to_string(pid)+ kStatusFilename);
  if(stream.is_open()){
  	while(std::getline(stream,line)){
    	
      std::istringstream linestream(line);
      linestream >> key;
      if(key == "Uid:"){
        linestream >> uid;
       
        break;
      
      }
    
    }
  
  }
  
  return uid; 
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) { 
  string uid = Uid(pid);
  string line, key, x, id;
  string user = "root";
  std::ifstream stream(kPasswordPath);
  if(stream.is_open()){
  	while(std::getline(stream,line)){
    	std::replace(line.begin(),line.end(),':',' ');
      	std::istringstream linestream(line);
      linestream >> key >> x >> id;
      if(id == uid){
      	user = key;
        break;
      }
    }
  }
  return user; 
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) { 
  string line, key;
  vector<string> values;
  long uptime;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if(stream.is_open()){
  	std::getline(stream,line);
    std::istringstream linestream(line);
    
    while(linestream >> key){
    	values.push_back(key);
    }
  }
  
  try{
	uptime = stol(values[21]) / sysconf(_SC_CLK_TCK);
  }catch(...){
    uptime = 0;
  }
  
  return uptime; 
}
