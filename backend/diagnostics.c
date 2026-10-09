
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <time.h>
#define CPU_WARNING 80.0
#define MEMORY_WARNING 80.0
#define DISK_WARNING 80.0

/* ---------- CPU ---------- */

void show_cpu()
{
    FILE *fp;
    char line[256];
    char model[256] = "Unknown";
    int processors = 0;

    fp = fopen("/proc/cpuinfo", "r");

    if (fp == NULL)
    {
        perror("Cannot open /proc/cpuinfo");
        return;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "model name", 10) == 0 && strcmp(model, "Unknown") == 0)
        {
            char *colon = strchr(line, ':');

            if (colon != NULL)
            {
                strcpy(model, colon + 2);
                model[strcspn(model, "\n")] = '\0';
            }
        }

        if (strncmp(line, "processor", 9) == 0)
            processors++;
    }

    fclose(fp);

    printf("\n========== CPU INFORMATION ==========\n");
    printf("CPU Model       : %s\n", model);
    printf("CPU Cores       : %d\n", processors);

    /* Get CPU usage using top */
    printf("CPU Usage       : ");

    fflush(stdout);

    system("top -bn1 | grep 'Cpu(s)' | awk '{print 100 - $8\"%\"}'");

    printf("=====================================\n");
}


/* ---------- MEMORY ---------- */

void show_memory()
{
    FILE *fp;
    char line[256];

    long total = 0;
    long available = 0;

    fp = fopen("/proc/meminfo", "r");

    if (fp == NULL)
    {
        perror("Cannot open /proc/meminfo");
        return;
    }

    while (fgets(line, sizeof(line), fp))
    {
        if (sscanf(line, "MemTotal: %ld kB", &total) == 1)
        {
            continue;
        }

        if (sscanf(line, "MemAvailable: %ld kB", &available) == 1)
        {
            continue;
        }
    }

    fclose(fp);

    if (total == 0)
    {
        printf("Memory information unavailable.\n");
        return;
    }

    long used = total - available;

    double usage = ((double)used / total) * 100.0;

    printf("\n========== MEMORY INFORMATION ==========\n");

    printf("Total Memory     : %.2f GB\n",
           total / (1024.0 * 1024.0));

    printf("Used Memory      : %.2f GB\n",
           used / (1024.0 * 1024.0));

    printf("Available Memory : %.2f GB\n",
           available / (1024.0 * 1024.0));

    printf("Memory Usage     : %.2f%%\n", usage);

    if (usage >= MEMORY_WARNING)
    {
        printf("WARNING: High memory usage detected!\n");
    }
    else
    {
        printf("Status           : Memory usage is normal.\n");
    }

    printf("=========================================\n");
}


/* ---------- DISK ---------- */

void show_disk()
{
    struct statvfs stat;

    if (statvfs("/", &stat) != 0)
    {
        perror("Cannot read disk information");
        return;
    }

    unsigned long long total =
        (unsigned long long)stat.f_blocks * stat.f_frsize;

    unsigned long long free_space =
        (unsigned long long)stat.f_bfree * stat.f_frsize;

    unsigned long long used =
        total - free_space;

    double usage = ((double)used / total) * 100.0;

    printf("\n========== DISK INFORMATION ==========\n");

    printf("Total Disk       : %.2f GB\n",
           total / (1024.0 * 1024.0 * 1024.0));

    printf("Used Disk        : %.2f GB\n",
           used / (1024.0 * 1024.0 * 1024.0));

    printf("Free Disk        : %.2f GB\n",
           free_space / (1024.0 * 1024.0 * 1024.0));

    printf("Disk Usage       : %.2f%%\n", usage);

    if (usage >= DISK_WARNING)
    {
        printf("WARNING: Very high disk usage detected!\n");
    }
    else
    {
        printf("Status           : Disk usage is normal.\n");
    }

    printf("=======================================\n");
}


/* ---------- PROCESS ---------- */

void show_processes()
{
    printf("\n========== RUNNING PROCESSES ==========\n");

    printf("Showing top running processes:\n\n");

    system("ps -eo pid,comm,state --sort=-pid | head -n 16");

    printf("\n=======================================\n");
}


/* ---------- NETWORK ---------- */

void show_network()
{
    printf("\n========== NETWORK INFORMATION ==========\n");

    system("ip -br addr");

    printf("\nNetwork Statistics:\n");

    system("cat /proc/net/dev");

    printf("=========================================\n");
}


/* ---------- UPTIME ---------- */

void show_uptime()
{
    FILE *fp;
    double uptime;

    fp = fopen("/proc/uptime", "r");

    if (fp == NULL)
    {
        perror("Cannot open /proc/uptime");
        return;
    }

    fscanf(fp, "%lf", &uptime);

    fclose(fp);

    int days = uptime / 86400;
    int hours = ((int)uptime % 86400) / 3600;
    int minutes = ((int)uptime % 3600) / 60;
    int seconds = (int)uptime % 60;

    printf("\n========== SYSTEM UPTIME ==========\n");

    printf("System Uptime : %d days %02d:%02d:%02d\n",
           days,
           hours,
           minutes,
           seconds);

    printf("===================================\n");
}


/* ---------- SYSTEM INFORMATION ---------- */

void show_system_info()
{
    printf("\n========== SYSTEM INFORMATION ==========\n");

    printf("\nOperating System:\n");
    system("uname -o");

    printf("\nKernel Version:\n");
    system("uname -r");

    printf("\nHostname:\n");
    system("hostname");

    printf("\nCurrent User:\n");
    system("whoami");

    printf("\nLogged-in Users:\n");
    system("who");

    printf("========================================\n");
}


/* ---------- ALERTS ---------- */

void show_alerts()
{
    FILE *fp;
    char line[256];

    long total = 0;
    long available = 0;

    fp = fopen("/proc/meminfo", "r");

    if (fp != NULL)
    {
        while (fgets(line, sizeof(line), fp))
        {
            sscanf(line, "MemTotal: %ld kB", &total);
            sscanf(line, "MemAvailable: %ld kB", &available);
        }

        fclose(fp);
    }

    double memory_usage = 0;

    if (total > 0)
    {
        memory_usage =
            ((double)(total - available) / total) * 100;
    }

    struct statvfs stat;

    double disk_usage = 0;

    if (statvfs("/", &stat) == 0)
    {
        unsigned long long total_disk =
            (unsigned long long)stat.f_blocks * stat.f_frsize;

        unsigned long long free_disk =
            (unsigned long long)stat.f_bfree * stat.f_frsize;

        unsigned long long used_disk =
            total_disk - free_disk;

        disk_usage =
            ((double)used_disk / total_disk) * 100;
    }

    printf("\n========== DIAGNOSTIC ALERTS ==========\n");

    printf("Memory Usage : %.2f%%\n", memory_usage);
    printf("Disk Usage   : %.2f%%\n", disk_usage);

    if (memory_usage >= MEMORY_WARNING)
    {
        printf("[WARNING] High memory usage detected!\n");
    }
    else
    {
        printf("[OK] Memory usage is normal.\n");
    }

    if (disk_usage >= DISK_WARNING)
    {
        printf("[CRITICAL] Very high disk usage detected!\n");
    }
    else
    {
        printf("[OK] Disk usage is normal.\n");
    }

    printf("========================================\n");
}


/* ---------- REPORT ---------- */

void generate_report()
{
    FILE *report;

    report = fopen("diagnostic_report.txt", "w");

    if (report == NULL)
    {
        perror("Cannot create report");
        return;
    }

    fprintf(report,
            "========================================\n");
    fprintf(report,
            "       LINUX DIAGNOSTIC REPORT\n");
    fprintf(report,
            "========================================\n\n");

    fprintf(report, "CPU INFORMATION\n");
    fprintf(report, "----------------\n");

    fflush(report);

    /* CPU information */
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");

    if (cpuinfo != NULL)
    {
        char line[256];
        int count = 0;

        while (fgets(line, sizeof(line), cpuinfo) && count < 5)
        {
            fprintf(report, "%s", line);
            count++;
        }

        fclose(cpuinfo);
    }

    fprintf(report, "\nMEMORY INFORMATION\n");
    fprintf(report, "------------------\n");

    FILE *meminfo = fopen("/proc/meminfo", "r");

    if (meminfo != NULL)
    {
        char line[256];
        int count = 0;

        while (fgets(line, sizeof(line), meminfo) &&
               count < 5)
        {
            fprintf(report, "%s", line);
            count++;
        }

        fclose(meminfo);
    }

    fprintf(report, "\nDISK INFORMATION\n");
    fprintf(report, "----------------\n");

    struct statvfs stat;

    if (statvfs("/", &stat) == 0)
    {
        unsigned long long total =
            (unsigned long long)stat.f_blocks * stat.f_frsize;

        unsigned long long free_space =
            (unsigned long long)stat.f_bfree * stat.f_frsize;

        unsigned long long used =
            total - free_space;

        fprintf(report,
                "Total Disk : %.2f GB\n",
                total / (1024.0 * 1024.0 * 1024.0));

        fprintf(report,
                "Used Disk  : %.2f GB\n",
                used / (1024.0 * 1024.0 * 1024.0));

        fprintf(report,
                "Free Disk  : %.2f GB\n",
                free_space / (1024.0 * 1024.0 * 1024.0));
    }

    fprintf(report, "\nRUNNING PROCESSES\n");
    fprintf(report, "-----------------\n");

    fflush(report);

    fprintf(report,
            "\nSYSTEM UPTIME\n");
    fprintf(report,
            "--------------\n");

    FILE *up = fopen("/proc/uptime", "r");

    if (up != NULL)
    {
        double uptime;

        fscanf(up, "%lf", &uptime);

        fprintf(report,
                "Uptime: %.0f seconds\n",
                uptime);

        fclose(up);
    }

    fprintf(report,
            "\nNETWORK INFORMATION\n");
    fprintf(report,
            "-------------------\n");

    fprintf(report,
            "Network information can be viewed using:\n");
    fprintf(report,
            "ip -br addr\n");

    fprintf(report,
            "\nWARNINGS\n");
    fprintf(report,
            "--------\n");

    fprintf(report,
            "Report generated by Linux System Diagnostics Toolkit.\n");

    fprintf(report,
            "\n========================================\n");

    fclose(report);

    printf("\nReport generated successfully!\n");
    printf("File: diagnostic_report.txt\n");
}


/* ---------- COMPLETE DIAGNOSTIC ---------- */

void show_all()
{
    printf("\n");
    printf("========================================\n");
    printf("    LINUX SYSTEM DIAGNOSTICS TOOLKIT\n");
    printf("========================================\n");

    show_cpu();
    show_memory();
    show_disk();
    show_processes();
    show_network();
    show_uptime();
    show_system_info();
    show_alerts();

    printf("\n========================================\n");
    printf("       DIAGNOSTIC COMPLETED\n");
    printf("========================================\n");
}


/* ---------- MENU ---------- */

void menu()
{
    printf("\n");
    printf("========================================\n");
    printf("     LINUX SYSTEM DIAGNOSTICS TOOLKIT\n");
    printf("========================================\n");

    printf("1. CPU Information\n");
    printf("2. Memory Information\n");
    printf("3. Disk Information\n");
    printf("4. Process Information\n");
    printf("5. Network Information\n");
    printf("6. System Uptime\n");
    printf("7. System Information\n");
    printf("8. Diagnostic Alerts\n");
    printf("9. Generate Report\n");
    printf("10. Run Complete Diagnostic\n");
    printf("0. Exit\n");

    printf("========================================\n");
    printf("Enter your choice: ");
}


/* ---------- MAIN ---------- */

int main()
{
    int choice;

    while (1)
    {
        menu();

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid input.\n");

            while (getchar() != '\n');

            continue;
        }

        switch (choice)
        {
            case 1:
                show_cpu();
                break;

            case 2:
                show_memory();
                break;

            case 3:
                show_disk();
                break;

            case 4:
                show_processes();
                break;

            case 5:
                show_network();
                break;

            case 6:
                show_uptime();
                break;

            case 7:
                show_system_info();
                break;

            case 8:
                show_alerts();
                break;

            case 9:
                generate_report();
                break;

            case 10:
                show_all();
                break;

            case 0:
                printf("\nExiting Linux Diagnostics Toolkit...\n");
                return 0;

            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    }

    return 0;
}
