#ifndef D_ENGINE_MODULATOR_H
#define D_ENGINE_MODULATOR_H

#include <unistd.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <stdio.h>

#include "MultiUrlRequestInfo.h"
#include "RpcMethod.h"
#include "console.h"
#include "prefs.h"
#include "DownloadEngine.h"
#include "Option.h"
#include "RequestGroupMan.h"

#define SPEED_MODI_RATIO 	(0.8)
#define SPEED_MIN_RATIO		(0.2)
#define MODULATION_INTERVAL (10)

#define CPU_RATIO_BIN  		"/lib/aria2/cpu_ratio"
#define MEM_RATIO_BIN  		"/lib/aria2/mem_ratio"
#define FLOW_LIMIT_BIN 		"/lib/aria2/flow_limit"

#define SYNC_FILE_BIN		"sync"
#define DROP_CACHE_BIN		"echo 1 > /proc/sys/vm/drop_caches"

#define CPU_RATIO_FILE  	"/var/run/cpu_ratio"
#define MEM_RATIO_FILE  	"/var/run/mem_ratio"
#define FLOW_LIMIT_FILE 	"/var/run/flow_limit"

namespace aria2 {
class DownloadEngine;
class MultiUrlRequestInfo;
class Option;
class RequestGroupMan;

struct ratio_data{
	int active;
	int idle;
	int total;
};

struct cpu_ratio{
	int system;
	int self;
};

struct mem_ratio{
//	int total;
//	int free;
//	int self;
	int ratio;
};

struct flow_stat{
	int upload;
	int download;
	int self;
};

class EngineModulator
{
public:
	EngineModulator(DownloadEngine* e);
	~EngineModulator();
	
private:
	static DownloadEngine* e_;
	
	static bool cpu_ratio_enable;
	static bool mem_ratio_enable;	
	static bool flow_limit_enable;

	static struct cpu_ratio cpu_ratio_;
	static struct mem_ratio mem_ratio_;
	static struct flow_stat flow_stat_;

	static void get_cpu_ratio();
	static void get_mem_ratio();
	static void get_flow_stat();

public:
	static void process();
};


DownloadEngine* EngineModulator::e_;
bool EngineModulator::cpu_ratio_enable;
bool EngineModulator::mem_ratio_enable;	
bool EngineModulator::flow_limit_enable;
struct cpu_ratio EngineModulator::cpu_ratio_;
struct mem_ratio EngineModulator::mem_ratio_;
struct flow_stat EngineModulator::flow_stat_;


EngineModulator::EngineModulator(DownloadEngine* e)
{
	e_ = e;
	cpu_ratio_enable = false;
	mem_ratio_enable = false;
	flow_limit_enable = false;

	memset(&cpu_ratio_, 0, sizeof(struct cpu_ratio));
	memset(&mem_ratio_, 0, sizeof(struct mem_ratio));
	memset(&flow_stat_, 0, sizeof(struct flow_stat));
}

EngineModulator::~EngineModulator(){}


void EngineModulator::get_cpu_ratio()
{
	struct ratio_data ratio_data_    = {0};
	struct ratio_data ratio_data_tmp = {0};
	
	FILE *f = NULL;
	
	if(NULL != (f = fopen(CPU_RATIO_FILE, "r")))
	{
		fscanf(f, "%d %d", &ratio_data_tmp.idle, &ratio_data_tmp.total);
		OFFLINE_DEBUG("idle = %d total = %d.", ratio_data_tmp.idle, ratio_data_tmp.total);
		fclose(f);
	}
	
	system(CPU_RATIO_BIN);
	
	if(NULL != (f = fopen(CPU_RATIO_FILE, "r")))
	{
		fscanf(f, "%d %d", &ratio_data_.idle, &ratio_data_.total);
		OFFLINE_DEBUG("idle = %d total = %d.", ratio_data_tmp.idle, ratio_data_tmp.total);
		fclose(f);
	}

	ratio_data_.active    = ratio_data_.total - ratio_data_.idle;
	ratio_data_tmp.active = ratio_data_tmp.total - ratio_data_tmp.idle;
	ratio_data_.active    = ratio_data_.active - ratio_data_tmp.active;
	ratio_data_.total	  = ratio_data_.total - ratio_data_tmp.total;
	
	if(ratio_data_.total)
		cpu_ratio_.system = ((long)ratio_data_.active * 100 / (long)ratio_data_.total);
	else
		cpu_ratio_.system = 0;
	if(100 == cpu_ratio_.system || cpu_ratio_.system <= 0)
		cpu_ratio_.system = e_->getOption()->getAsInt(PREF_MAX_CPU_RATIO);
	OFFLINE_DEBUG("cpu_ratio_.system = %d.", cpu_ratio_.system);
}

void EngineModulator::get_mem_ratio()
{
	struct ratio_data ratio_data_    = {0};
	FILE *f = NULL;
	
	system(MEM_RATIO_BIN);

	if(NULL != (f = fopen(MEM_RATIO_FILE, "r")))
	{
		fscanf(f, "%d %d", &ratio_data_.total, &ratio_data_.idle);
		OFFLINE_DEBUG("idle = %d total = %d.", ratio_data_.idle, ratio_data_.total);
		fclose(f);
	}

	if(ratio_data_.total)
		mem_ratio_.ratio = ratio_data_.idle / ratio_data_.total;
	if(0 == mem_ratio_.ratio)
		mem_ratio_.ratio = e_->getOption()->getAsInt(PREF_MAX_MEM_RATIO);
}

void EngineModulator::get_flow_stat()
{
	;
}


void EngineModulator::process()
{
	
	bool cpu_ratio_enable;
	bool mem_ratio_enable;	
	bool flow_limit_enable;

    while(true)
    {
    	sleep(MODULATION_INTERVAL);

		cpu_ratio_enable  = e_->getOption()->getAsBool(PREF_ENGINE_CPU_MODULATE);
		mem_ratio_enable  = e_->getOption()->getAsBool(PREF_ENGINE_MEM_MODULATE);	
		flow_limit_enable = e_->getOption()->getAsBool(PREF_ENGINE_FLOW_MODULATE);

	  OFFLINE_DEBUG("==============================CPU ratio modulation below==============================");
		/* CPU ratio modulation. */
		if(cpu_ratio_enable)
		{
			OFFLINE_DEBUG("cpu_ratio_enable is true");
			get_cpu_ratio();
			if(cpu_ratio_.system > e_->getOption()->getAsInt(PREF_MAX_CPU_RATIO))
			{
				OFFLINE_DEBUG("NEED TO SPEED DOWNDOWNDOWNDOWNDOWNDOWNDOWNDOWN");
				OFFLINE_DEBUG("PREF_MAX_CPU_RATIO = %d",e_->getOption()->getAsInt(PREF_MAX_CPU_RATIO));
				
				OFFLINE_DEBUG("------Before Modulation------");
				OFFLINE_DEBUG("DOWNLOAD_LIMIT = %d",e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
				OFFLINE_DEBUG("UPLOAD_LIMIT   = %d",e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());

				/* Download Speed Limit. */
				if(e_->getOption()->defined(PREF_MAX_OVERALL_DOWNLOAD_LIMIT) && 
					e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit() >= e_->getOption()->getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT) * SPEED_MIN_RATIO) 
				{
					e_->getRequestGroupMan()->setMaxOverallDownloadSpeedLimit
				      (e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit() * SPEED_MODI_RATIO);
				}

				/* Upload Speed Limit. */
				if(e_->getOption()->defined(PREF_MAX_OVERALL_UPLOAD_LIMIT) && 
					e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit() >= e_->getOption()->getAsInt(PREF_MAX_OVERALL_UPLOAD_LIMIT) * SPEED_MIN_RATIO) 
				{
				    e_->getRequestGroupMan()->setMaxOverallUploadSpeedLimit
				      (e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit() * SPEED_MODI_RATIO);
  				}
				OFFLINE_DEBUG("------After Modulation------");
				OFFLINE_DEBUG("DOWNLOAD_LIMIT = %d",e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
				OFFLINE_DEBUG("UPLOAD_LIMIT   = %d",e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());
			}
			else if(cpu_ratio_.system <= (e_->getOption()->getAsInt(PREF_MAX_CPU_RATIO) * SPEED_MODI_RATIO))
			{
				OFFLINE_DEBUG("NEED TO SPEED UPUPUPUPUPUPUPUPUPUPUPUPUPUPUPUP");
				OFFLINE_DEBUG("CPU_RATIO * LIMIT_RATIO = %d",e_->getOption()->getAsInt(PREF_MAX_CPU_RATIO) * SPEED_MODI_RATIO);
				
				OFFLINE_DEBUG("------Before Modulation------");
				OFFLINE_DEBUG("DOWNLOAD_LIMIT = %d",e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
				OFFLINE_DEBUG("UPLOAD_LIMIT   = %d",e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());

				/* Download Speed Limit. */
				if(e_->getOption()->defined(PREF_MAX_OVERALL_DOWNLOAD_LIMIT) && 
					e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit() < e_->getOption()->getAsInt(PREF_MAX_OVERALL_DOWNLOAD_LIMIT) * SPEED_MIN_RATIO) 
				{
				    e_->getRequestGroupMan()->setMaxOverallDownloadSpeedLimit
				      (e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit() / SPEED_MODI_RATIO);
					OFFLINE_DEBUG("DOWNLOAD_LIMIT = %d", e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
				}

				/* Upload Speed Limit. */
				if(e_->getOption()->defined(PREF_MAX_OVERALL_UPLOAD_LIMIT) && 
					e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit() < e_->getOption()->getAsInt(PREF_MAX_OVERALL_UPLOAD_LIMIT) * SPEED_MIN_RATIO) 
				{
					e_->getRequestGroupMan()->setMaxOverallUploadSpeedLimit
				      (e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit() / SPEED_MODI_RATIO);
					OFFLINE_DEBUG("UPLOAD_LIMIT = %d", e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());
  				}
				OFFLINE_DEBUG("------After Modulation------");
				OFFLINE_DEBUG("DOWNLOAD_LIMIT = %d",e_->getRequestGroupMan()->getMaxOverallDownloadSpeedLimit());
				OFFLINE_DEBUG("UPLOAD_LIMIT   = %d",e_->getRequestGroupMan()->getMaxOverallUploadSpeedLimit());
			}
		}
		OFFLINE_DEBUG("==============================end CPU ratio modulation==============================\n");
		
		/* MEM ratio modulation. */
		
		if(mem_ratio_enable)
		{
			get_mem_ratio();

			if(mem_ratio_.ratio < e_->getOption()->getAsInt(PREF_MAX_MEM_RATIO))
			{
				OFFLINE_DEBUG("==========DROP CACHE BELOW===========");
				system(SYNC_FILE_BIN);
				system(DROP_CACHE_BIN);
				OFFLINE_DEBUG("========DROP CACHE FINISHED==========");
			}
		}
		
		/* FLOW limit modulation. */
		
		if(flow_limit_enable)
		{
			get_flow_stat();
		}
	
    }
}

}
#endif