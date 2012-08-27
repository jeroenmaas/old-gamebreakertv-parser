
#include "stdafx.h"
#include "Log.h"
#include <time.h>

CLog::CLog()
{
#ifdef WIN32
	stderr_handle = GetStdHandle(STD_ERROR_HANDLE);
	stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

    m_logLevel = 0;
    m_logFileLevel = 0;
    m_time = false;
    logfile = NULL;
}

void CLog::Init(int32 console_level, int32 logFileLevel, bool time, FILE* log)
{
    m_logLevel = console_level;
    m_logFileLevel = logFileLevel;
    m_time = time;
    logfile = log;
    if(log)
        PrintStartupInformationToLog();
}

void CLog::Color(unsigned int color)
{
#ifndef WIN32
	static const char* colorstrings[TBLUE+1] = {
		"",
			"\033[22;31m",
			"\033[22;32m",
			"\033[01;33m",
			//"\033[22;37m",
			"\033[0m",
			"\033[01;37m",
			"\033[1;34m",
	};
	fputs(colorstrings[color], stdout);
#else
    SetConsoleTextAttribute(stdout_handle, (WORD)color);
#endif
}

void CLog::Time()
{
    if(m_time)
    {
        tm local;
        time_t curr;
        time(&curr);
        local=*(localtime(&curr)); 
	    printf("%02u:%02u ", local.tm_hour, local.tm_min);
    }
}

void CLog::Notice(const char * source, const char * format, ...)
{
	/* notice is old loglevel 0/string */
	va_list ap;
	va_start(ap, format);
	Time();
    Color(TWHITE);
	fputs("N ", stdout);
	if(source != NULL && *source)
	{
		Color(TWHITE);
		fputs(source, stdout);
		putchar(':');
		putchar(' ');
		Color(TWHITE);
	}

	vprintf(format, ap);
	putchar('\n');
	va_end(ap);
	Color(TNORMAL);

    if (logfile)
    {
        va_list ap;
        outTimestamp();
        fprintf(logfile, source );
        fprintf(logfile, ": " );
        va_start(ap, format);
        vfprintf(logfile, format, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
}

void CLog::Line()
{
	putchar('\n');
}

void CLog::Error(const char * source, const char * format, ...)
{
	if( m_logLevel >= LOG_LVL_BASIC )
    {
	    va_list ap;
	    va_start(ap, format);
	    Time();
	    Color(TRED);
	    fputs("E ", stdout);
	    if(*source)
	    {
		    Color(TWHITE);
		    fputs(source, stdout);
		    putchar(':');
		    putchar(' ');
		    Color(TRED);
	    }

	    vprintf(format, ap);
	    putchar('\n');
	    va_end(ap);
	    Color(TNORMAL);
    }

    if (logfile && m_logFileLevel >= LOG_LVL_BASIC)
    {
        va_list ap;
        outTimestamp();
        fprintf(logfile, source );
        fprintf(logfile, ": " );
        va_start(ap, format);
        vfprintf(logfile, format, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
}

void CLog::Warning(const char * source, const char * format, ...)
{
	if(m_logLevel >= LOG_LVL_DETAIL)
    {
	    /* warning is old loglevel 2/detail */
	    va_list ap;
	    va_start(ap, format);
	    Time();
	    Color(TYELLOW);
	    fputs("W ", stdout);
	    if(source != NULL && *source)
	    {
		    Color(TWHITE);
		    fputs(source, stdout);
		    putchar(':');
		    putchar(' ');
		    Color(TYELLOW);
	    }

	    vprintf(format, ap);
	    putchar('\n');
	    va_end(ap);
	    Color(TNORMAL);
    }

    if (logfile && m_logFileLevel >= LOG_LVL_DETAIL)
    {
        va_list ap;
        outTimestamp();
        fprintf(logfile, source );
        fprintf(logfile, ": " );
        va_start(ap, format);
        vfprintf(logfile, format, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
}

void CLog::Success(const char * source, const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	Time();
	Color(TGREEN);
	fputs("S ", stdout);
	if(source != NULL && *source)
	{
		Color(TWHITE);
		fputs(source, stdout);
		putchar(':');
		putchar(' ');
		Color(TGREEN);
	}

	vprintf(format, ap);
	putchar('\n');
	va_end(ap);
	Color(TNORMAL);

    if (logfile)
    {
        va_list ap;
        outTimestamp();
        fprintf(logfile, source );
        fprintf(logfile, ": " );
        va_start(ap, format);
        vfprintf(logfile, format, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
}

void CLog::Debug(const char * source, const char * format, ...)
{
	if(m_logLevel >= LOG_LVL_DEBUG)
    {
	    va_list ap;
	    va_start(ap, format);
	    Time();
	    Color(TBLUE);
	    fputs("D ", stdout);
	    if(source != NULL && *source)
	    {
		    Color(TWHITE);
		    fputs(source, stdout);
		    putchar(':');
		    putchar(' ');
		    Color(TBLUE);
	    }

	    vprintf(format, ap);
	    putchar('\n');
	    va_end(ap);
	    Color(TNORMAL);
    }

    if (logfile && m_logFileLevel >= LOG_LVL_DEBUG)
    {
        va_list ap;
        outTimestamp();
        fprintf(logfile, source );
        fprintf(logfile, ": " );
        va_start(ap, format);
        vfprintf(logfile, format, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
}

void CLog::String(const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	Time();
	vprintf(format, ap);
	putchar('\n');
	va_end(ap);

    if (logfile)
    {
        va_list ap;
        outTimestamp();
        va_start(ap, format);
        vfprintf(logfile, format, ap);
        fprintf(logfile, "\n" );
        va_end(ap);
        fflush(logfile);
    }
}

void CLog::LargeErrorMessage(uint32 Colour, ...)
{
	std::vector<char*> lines;
	char * pointer;
	va_list ap;
	va_start(ap, Colour);
		
	size_t i,j,k;
	pointer = va_arg(ap, char*);
	while( pointer != NULL )
	{
		lines.push_back( pointer );
		pointer = va_arg(ap, char*);
	}

	if( Colour == LARGERRORMESSAGE_ERROR )
		Color(TRED);
	else
		Color(TYELLOW);

    printf("\n");
	printf("*********************************************************************\n");
	printf("*                        MAJOR ERROR/WARNING                        *\n");
	printf("*                        ===================                        *\n");

	for(std::vector<char*>::iterator itr = lines.begin(); itr != lines.end(); ++itr)
	{
		i = strlen(*itr);
		j = (i<=65) ? 65 - i : 0;

		printf("* %s", *itr);
		for( k = 0; k < j; ++k )
		{
			printf(" ");
		}

		printf(" *\n");
	}

	printf("*********************************************************************\n");

	Sleep(50000);

	Color(TNORMAL);
}

void CLog::outTimestamp()
{
    time_t t = time(NULL);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    fprintf(logfile,"%-4d-%02d-%02d %02d:%02d:%02d ",aTm->tm_year+1900,aTm->tm_mon+1,aTm->tm_mday,aTm->tm_hour,aTm->tm_min,aTm->tm_sec);
}

void CLog::PrintStartupInformationToLog()
{
    fprintf(logfile,"\n");
    outTimestamp();
    fprintf(logfile, "Extractor Starting. \n" );
}