#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MAX_STUDENTS 60

/* ─────────────────────────────────────────
   STRUCTURES
───────────────────────────────────────── */

struct student
{
    int regno;
    char name[50];
};

struct attendance_stat
{
    int regno;
    char name[50];
    float percentage;
};

struct day_stat
{
    char date[11];
    int present_count;
    int absent_count;
};

/* ─────────────────────────────────────────
   GLOBALS
───────────────────────────────────────── */

struct student students[MAX_STUDENTS];
struct attendance_stat stats[MAX_STUDENTS];
int student_count = 0;

/* ─────────────────────────────────────────
   FUNCTION PROTOTYPES  (hierarchical order)
   
   Tier 1 — Low-level input / validation
   Tier 2 — File I/O helpers
   Tier 3 — Core student operations
   Tier 4 — Attendance operations
   Tier 5 — Reporting / analysis
───────────────────────────────────────── */

/* Tier 1 — Input & validation */
int         read_regno();
int         read_menu_choice();
char        read_status();
int         validate_date(char date[]);
int         validate_name(char name[]);
void        normalize_name(char name[]);

/* Tier 2 — File I/O helpers */
void        load_students();
void        save_students();
void        remove_existing_date(char date[]);

/* Tier 3 — Core student operations */
int         find_student_index(int regno);
void        add_student();
void        display_students();
void        modify_student();
void        delete_student();

/* Tier 4 — Attendance operations */
void        mark_attendance();
void        modify_attendance();
float       calculate_percentage_for_student(int regno);
void        calculate_all_percentages();

/* Tier 5 — Reporting & analysis */
void        show_all_percentages();
void        sort_by_percentage();
void        list_by_category();
void        show_day_wise_attendance();
void        view_student_attendance_history();
void        generate_analysis_report();

/* ─────────────────────────────────────────
   TIER 1 — INPUT & VALIDATION
───────────────────────────────────────── */

int read_regno()
{
    int regno;

    while (1)
    {
        int r = scanf("%d", &regno);

        if (r == EOF)
        {
            clearerr(stdin);
            return -1;
        }

        if (r != 1)
        {
            printf("Invalid input. Enter numeric register number: ");
            fflush(stdout);
            int c; while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (regno <= 0)
        {
            printf("Register number must be positive: ");
            fflush(stdout);
            continue;
        }

        return regno;
    }
}

int read_menu_choice()
{
    int choice;
    int r = scanf("%d", &choice);

    if (r == EOF)
    {
        clearerr(stdin);
        return -1;
    }

    if (r != 1)
    {
        printf("Invalid input. Enter a number.\n");
        fflush(stdout);
        int c; while ((c = getchar()) != '\n' && c != EOF);
        return -1;
    }

    return choice;
}

char read_status()
{
    char status;

    while (1)
    {
        int r = scanf(" %c", &status);

        if (r == EOF)
        {
            clearerr(stdin);
            return 'A';
        }

        if (status == 'P' || status == 'p')
            return 'P';

        if (status == 'A' || status == 'a')
            return 'A';

        printf("Invalid input. Enter P or A: ");
        fflush(stdout);
    }
}

int validate_date(char date[])
{
    int day, month, year;

    if (sscanf(date, "%2d-%2d-%4d", &day, &month, &year) != 3)
        return 0;

    if (day < 1 || day > 31)
        return 0;

    if (month < 1 || month > 12)
        return 0;

    if (year < 2000 || year > 2100)
        return 0;

    return 1;
}

int validate_name(char name[])
{
    int i;

    for (i = 0; name[i] != '\0'; i++)
    {
        if (!(name[i] >= 'A' && name[i] <= 'Z') &&
            !(name[i] >= 'a' && name[i] <= 'z') &&
            name[i] != ' ')
        {
            return 0;
        }
    }

    return 1;
}

/* Converts all characters in name to uppercase */
void normalize_name(char name[])
{
    int i = 0;

    while (name[i] != '\0')
    {
        if (name[i] >= 'a' && name[i] <= 'z')
            name[i] = name[i] - 32;
        i++;
    }
}

/* ─────────────────────────────────────────
   TIER 2 — FILE I/O HELPERS
───────────────────────────────────────── */

void load_students()
{
    FILE *fp;

    fp = fopen("students.txt", "r");

    if (fp == NULL)
    {
        printf("No existing student file found. Starting fresh.\n");
        fflush(stdout);
        return;
    }

    while (fscanf(fp, "%d,%49[^\n]\n",
                  &students[student_count].regno,
                  students[student_count].name) == 2)
    {
        if (student_count >= MAX_STUDENTS)
        {
            printf("Student limit reached while loading file.\n");
            fflush(stdout);
            break;
        }

        student_count++;
    }

    fclose(fp);
}

void save_students()
{
    FILE *fp;
    int i;

    fp = fopen("students.txt", "w");

    if (fp == NULL)
    {
        printf("Error saving student file.\n");
        fflush(stdout);
        return;
    }

    for (i = 0; i < student_count; i++)
    {
        fprintf(fp, "%d,%s\n",
                students[i].regno,
                students[i].name);
    }

    fclose(fp);
}

void remove_existing_date(char date[])
{
    FILE *fp, *temp;
    char d[11];
    int r;
    char status;

    fp = fopen("attendance.txt", "r");

    if (fp == NULL)
        return;

    temp = fopen("temp.txt", "w");

    while (fscanf(fp, "%10[^,],%d,%c\n", d, &r, &status) == 3)
    {
        if (strcmp(d, date) != 0)
            fprintf(temp, "%s,%d,%c\n", d, r, status);
    }

    fclose(fp);
    fclose(temp);

    remove("attendance.txt");
    rename("temp.txt", "attendance.txt");
}

/* ─────────────────────────────────────────
   TIER 3 — CORE STUDENT OPERATIONS
───────────────────────────────────────── */

int find_student_index(int regno)
{
    int i;

    for (i = 0; i < student_count; i++)
    {
        if (students[i].regno == regno)
            return i;
    }

    return -1;
}

void add_student()
{
    int regno;

    if (student_count >= MAX_STUDENTS)
    {
        printf("Maximum student limit reached.\n");
        fflush(stdout);
        return;
    }

    printf("Enter register number: ");
    fflush(stdout);
    regno = read_regno();

    if (find_student_index(regno) != -1)
    {
        printf("Student with this register number already exists.\n");
        fflush(stdout);
        return;
    }

    students[student_count].regno = regno;

    printf("Enter name: ");
    fflush(stdout);
    scanf(" %[^\n]", students[student_count].name);

    while (!validate_name(students[student_count].name))
    {
        printf("Invalid name. Enter alphabetic name: ");
        fflush(stdout);
        scanf(" %[^\n]", students[student_count].name);
    }

    normalize_name(students[student_count].name);

    student_count++;

    save_students();

    printf("Student added successfully.\n");
    fflush(stdout);
}

void display_students()
{
    int i;

    if (student_count == 0)
    {
        printf("No students available.\n");
        fflush(stdout);
        return;
    }

    printf("\nStudent List:\n");
    fflush(stdout);

    for (i = 0; i < student_count; i++)
    {
        printf("RegdNo: %d  Name: %s\n",
               students[i].regno,
               students[i].name);
        fflush(stdout);
    }
}

void modify_student()
{
    int regno;
    int index;

    printf("Enter register number to modify: ");
    fflush(stdout);
    regno = read_regno();

    index = find_student_index(regno);

    if (index == -1)
    {
        printf("Student not found.\n");
        fflush(stdout);
        return;
    }

    printf("Current name: %s\n", students[index].name);
    fflush(stdout);

    printf("Enter new name: ");
    fflush(stdout);
    scanf(" %[^\n]", students[index].name);

    while (!validate_name(students[index].name))
    {
        printf("Invalid name. Enter alphabetic name: ");
        fflush(stdout);
        scanf(" %[^\n]", students[index].name);
    }

    normalize_name(students[index].name);

    save_students();

    printf("Student updated successfully.\n");
    fflush(stdout);
}

void delete_student()
{
    int regno;
    int index;
    int i;

    printf("Enter register number to delete: ");
    fflush(stdout);
    regno = read_regno();

    index = find_student_index(regno);

    if (index == -1)
    {
        printf("Student not found.\n");
        fflush(stdout);
        return;
    }

    for (i = index; i < student_count - 1; i++)
        students[i] = students[i + 1];

    student_count--;

    save_students();

    printf("Student removed from student list.\n");
    fflush(stdout);

    FILE *fp, *temp;
    char date[11];
    int r;
    char status;

    fp = fopen("attendance.txt", "r");

    if (fp == NULL)
        return;

    temp = fopen("temp.txt", "w");

    while (fscanf(fp, "%10[^,],%d,%c\n", date, &r, &status) == 3)
    {
        if (r != regno)
            fprintf(temp, "%s,%d,%c\n", date, r, status);
    }

    fclose(fp);
    fclose(temp);

    remove("attendance.txt");
    rename("temp.txt", "attendance.txt");

    printf("Associated attendance records deleted.\n");
    fflush(stdout);
}

/* ─────────────────────────────────────────
   TIER 4 — ATTENDANCE OPERATIONS
───────────────────────────────────────── */

void mark_attendance()
{
    char date[11];
    char status;
    int i;

    if (student_count == 0)
    {
        printf("No students available. Add students first.\n");
        fflush(stdout);
        return;
    }

    printf("Enter Date (dd-mm-yyyy): ");
    fflush(stdout);
    scanf("%s", date);

    while (!validate_date(date))
    {
        printf("Invalid date format. Enter (dd-mm-yyyy): ");
        fflush(stdout);
        scanf("%s", date);
    }

    FILE *check = fopen("attendance.txt", "r");
    if (check != NULL)
    {
        char d[11];
        int r;
        char s;
        int exists = 0;

        while (fscanf(check, "%10[^,],%d,%c\n", d, &r, &s) == 3)
        {
            if (strcmp(d, date) == 0)
            {
                exists = 1;
                break;
            }
        }

        fclose(check);

        if (exists)
        {
            printf("Warning: Attendance for %s already exists. Overwriting.\n", date);
            fflush(stdout);
        }
    }

    remove_existing_date(date);

    FILE *fp = fopen("attendance.txt", "a");

    if (fp == NULL)
    {
        printf("Error opening attendance file.\n");
        fflush(stdout);
        return;
    }

    for (i = 0; i < student_count; i++)
    {
        float perc = calculate_percentage_for_student(students[i].regno);

        printf("\nRegdNo: %d | Name: %s | Current %%: %.2f\n",
               students[i].regno,
               students[i].name,
               perc);
        fflush(stdout);

        printf("Enter Status (P/A): ");
        fflush(stdout);
        status = read_status();

        fprintf(fp, "%s,%d,%c\n",
                date,
                students[i].regno,
                status);
    }

    fclose(fp);

    printf("Attendance recorded successfully.\n");
    fflush(stdout);
}

void modify_attendance()
{
    FILE *fp, *temp;

    char date[11];
    char d[11];
    int regno, r;
    char status, new_status;

    int found = 0;

    printf("Enter date (dd-mm-yyyy): ");
    fflush(stdout);
    scanf("%s", date);

    while (!validate_date(date))
    {
        printf("Invalid date format. Enter (dd-mm-yyyy): ");
        fflush(stdout);
        scanf("%s", date);
    }

    printf("Enter register number: ");
    fflush(stdout);
    regno = read_regno();

    fp = fopen("attendance.txt", "r");

    if (fp == NULL)
    {
        printf("Attendance file not found.\n");
        fflush(stdout);
        return;
    }

    temp = fopen("temp.txt", "w");

    while (fscanf(fp, "%10[^,],%d,%c\n", d, &r, &status) == 3)
    {
        if (strcmp(d, date) == 0 && r == regno)
        {
            printf("Current Status: %c\n", status);
            fflush(stdout);
            printf("Enter new status (P/A): ");
            fflush(stdout);
            new_status = read_status();

            fprintf(temp, "%s,%d,%c\n", d, r, new_status);
            found = 1;
        }
        else
        {
            fprintf(temp, "%s,%d,%c\n", d, r, status);
        }
    }

    fclose(fp);
    fclose(temp);

    remove("attendance.txt");
    rename("temp.txt", "attendance.txt");

    if (found)
    {
        printf("Attendance updated successfully.\n");
        fflush(stdout);
    }
    else
    {
        printf("Record not found.\n");
        fflush(stdout);
    }
}

float calculate_percentage_for_student(int regno)
{
    FILE *fp;
    char date[11];
    int r;
    char status;

    int present_days = 0;

    char unique_dates[200][11];
    int unique_count = 0;

    fp = fopen("attendance.txt", "r");

    if (fp == NULL)
        return 0;

    while (fscanf(fp, "%10[^,],%d,%c\n", date, &r, &status) == 3)
    {
        int i;
        int found = 0;

        for (i = 0; i < unique_count; i++)
        {
            if (strcmp(unique_dates[i], date) == 0)
            {
                found = 1;
                break;
            }
        }

        if (!found)
        {
            strcpy(unique_dates[unique_count], date);
            unique_count++;
        }

        if (r == regno && status == 'P')
            present_days++;
    }

    fclose(fp);

    if (unique_count == 0)
        return 0;

    return (present_days * 100.0) / unique_count;
}

void calculate_all_percentages()
{
    int i;

    for (i = 0; i < student_count; i++)
    {
        stats[i].regno = students[i].regno;
        strcpy(stats[i].name, students[i].name);
        stats[i].percentage = calculate_percentage_for_student(students[i].regno);
    }
}

/* ─────────────────────────────────────────
   TIER 5 — REPORTING & ANALYSIS
───────────────────────────────────────── */

void show_all_percentages()
{
    int i;

    calculate_all_percentages();

    printf("\nAttendance Percentages:\n");
    fflush(stdout);

    for (i = 0; i < student_count; i++)
    {
        printf("RegdNo: %d | Name: %s | Attendance: %.2f%%\n",
               stats[i].regno,
               stats[i].name,
               stats[i].percentage);
        fflush(stdout);
    }
}

void sort_by_percentage()
{
    int i, j;
    struct attendance_stat temp;

    calculate_all_percentages();

    for (i = 0; i < student_count - 1; i++)
    {
        for (j = 0; j < student_count - i - 1; j++)
        {
            if (stats[j].percentage < stats[j + 1].percentage)
            {
                temp = stats[j];
                stats[j] = stats[j + 1];
                stats[j + 1] = temp;
            }
        }
    }

    printf("\nStudents sorted by attendance percentage:\n");
    fflush(stdout);

    for (i = 0; i < student_count; i++)
    {
        printf("RegdNo: %d | Name: %s | Attendance: %.2f%%\n",
               stats[i].regno,
               stats[i].name,
               stats[i].percentage);
        fflush(stdout);
    }
}

void list_by_category()
{
    int i;

    calculate_all_percentages();

    printf("\n===== Shortage Students (<75%%) =====\n");
    fflush(stdout);

    for (i = 0; i < student_count; i++)
    {
        if (stats[i].percentage < 75)
        {
            printf("RegdNo: %d | Name: %s | %.2f%%  *** SHORTAGE ***\n",
                   stats[i].regno,
                   stats[i].name,
                   stats[i].percentage);
            fflush(stdout);
        }
    }

    printf("\n===== At Risk Students (75-80%%) =====\n");
    fflush(stdout);

    for (i = 0; i < student_count; i++)
    {
        if (stats[i].percentage >= 75 && stats[i].percentage <= 80)
        {
            printf("RegdNo: %d | Name: %s | %.2f%%  WARNING\n",
                   stats[i].regno,
                   stats[i].name,
                   stats[i].percentage);
            fflush(stdout);
        }
    }

    printf("\n===== Safe Students (>80%%) =====\n");
    fflush(stdout);

    for (i = 0; i < student_count; i++)
    {
        if (stats[i].percentage > 80)
        {
            printf("RegdNo: %d | Name: %s | %.2f%%\n",
                   stats[i].regno,
                   stats[i].name,
                   stats[i].percentage);
            fflush(stdout);
        }
    }
}

void show_day_wise_attendance()
{
    FILE *fp;

    char date[11];
    int regno;
    char status;

    struct day_stat days[200];
    int day_count = 0;

    int i;

    fp = fopen("attendance.txt", "r");

    if (fp == NULL)
    {
        printf("No attendance data available.\n");
        fflush(stdout);
        return;
    }

    while (fscanf(fp, "%10[^,],%d,%c\n", date, &regno, &status) == 3)
    {
        int found = -1;

        for (i = 0; i < day_count; i++)
        {
            if (strcmp(days[i].date, date) == 0)
            {
                found = i;
                break;
            }
        }

        if (found == -1)
        {
            strcpy(days[day_count].date, date);
            days[day_count].present_count = 0;
            days[day_count].absent_count = 0;
            found = day_count;
            day_count++;
        }

        if (status == 'P')
            days[found].present_count++;
        else
            days[found].absent_count++;
    }

    fclose(fp);

    printf("\n===== Day-Wise Attendance =====\n");
    fflush(stdout);

    for (i = 0; i < day_count; i++)
    {
        printf("%s - Present: %d | Absent: %d\n",
               days[i].date,
               days[i].present_count,
               days[i].absent_count);
        fflush(stdout);
    }
}

void view_student_attendance_history()
{
    int regno;
    int index;

    FILE *fp;

    char date[11];
    int r;
    char status;

    printf("Enter register number: ");
    fflush(stdout);
    regno = read_regno();

    index = find_student_index(regno);

    if (index == -1)
    {
        printf("Student not found.\n");
        fflush(stdout);
        return;
    }

    fp = fopen("attendance.txt", "r");

    if (fp == NULL)
    {
        printf("No attendance data available.\n");
        fflush(stdout);
        return;
    }

    printf("\nAttendance History for %s\n\n", students[index].name);
    fflush(stdout);

    while (fscanf(fp, "%10[^,],%d,%c\n", date, &r, &status) == 3)
    {
        if (r == regno)
        {
            printf("%s - %c\n", date, status);
            fflush(stdout);
        }
    }

    fclose(fp);

    float perc = calculate_percentage_for_student(regno);

    printf("\nAttendance Percentage: %.2f%%\n", perc);
    fflush(stdout);
}

void generate_analysis_report()
{
    FILE *fp, *report;

    char date[11];
    int regno;
    char status;

    struct day_stat days[200];
    int day_count = 0;

    int i;

    fp = fopen("attendance.txt", "r");

    if (fp == NULL)
    {
        printf("No attendance data available.\n");
        fflush(stdout);
        return;
    }

    while (fscanf(fp, "%10[^,],%d,%c\n", date, &regno, &status) == 3)
    {
        int found = -1;

        for (i = 0; i < day_count; i++)
        {
            if (strcmp(days[i].date, date) == 0)
            {
                found = i;
                break;
            }
        }

        if (found == -1)
        {
            strcpy(days[day_count].date, date);
            days[day_count].present_count = 0;
            days[day_count].absent_count = 0;
            found = day_count;
            day_count++;
        }

        if (status == 'P')
            days[found].present_count++;
        else
            days[found].absent_count++;
    }

    fclose(fp);

    report = fopen("analysis_report.txt", "w");

    if (report == NULL)
    {
        printf("Error creating report file.\n");
        fflush(stdout);
        return;
    }

    calculate_all_percentages();

    struct attendance_stat top[MAX_STUDENTS];
    int top_count = student_count;

    for(i = 0; i < top_count; i++)
    {
        top[i] = stats[i];
    }

    int j;
    struct attendance_stat temp;

    for(i = 0; i < top_count - 1; i++)
    {
        for(j = 0; j < top_count - i - 1; j++)
        {
            if(top[j].percentage < top[j+1].percentage)
            {
                temp = top[j];
                top[j] = top[j+1];
                top[j+1] = temp;
            }
        }
    }

    float class_avg = 0;
    int shortage = 0, risk = 0, safe = 0;

    for (i = 0; i < student_count; i++)
    {
        class_avg += stats[i].percentage;

        if (stats[i].percentage < 75)
            shortage++;
        else if (stats[i].percentage <= 80)
            risk++;
        else
            safe++;
    }

    if (student_count > 0)
        class_avg = class_avg / student_count;
    else
        class_avg = 0;

    fprintf(report, "===== Attendance Analysis Report =====\n\n");
    fprintf(report, "Total Students: %d\n", student_count);
    fprintf(report, "Total Working Days: %d\n", day_count);
    fprintf(report, "Class Average Attendance: %.2f%%\n\n", class_avg);

    fprintf(report, "Category Breakdown:\n");
    fprintf(report, "Shortage (<75%%): %d\n", shortage);
    fprintf(report, "At Risk (75-80%%): %d\n", risk);
    fprintf(report, "Safe (>80%%): %d\n\n", safe);

    fprintf(report, "Daily Attendance:\n");

    int max_absent = -1;
    int max_index = -1;

    for (i = 0; i < day_count; i++)
    {
        float perc = 0;

        if (student_count > 0)
            perc = (days[i].present_count * 100.0) / student_count;

        fprintf(report,
                "%s - Present: %d | Absent: %d | Attendance: %.2f%%\n",
                days[i].date,
                days[i].present_count,
                days[i].absent_count,
                perc);

        if (days[i].absent_count > max_absent)
        {
            max_absent = days[i].absent_count;
            max_index = i;
        }
    }

    fprintf(report, "\nTop Performers:\n");

    int limit = 3;

    if(top_count < 3)
        limit = top_count;

    for(i = 0; i < limit; i++)
    {
        fprintf(report,
                "%d. %s (RegdNo: %d) - %.2f%%\n",
                i+1,
                top[i].name,
                top[i].regno,
                top[i].percentage);
    }

    if (max_index != -1)
    {
        fprintf(report,
                "\nMost absent day: %s (%d absentees)\n",
                days[max_index].date,
                days[max_index].absent_count);
    }

    fclose(report);

    printf("Analysis report generated: analysis_report.txt\n");
    fflush(stdout);
}

/* ─────────────────────────────────────────
   MAIN
───────────────────────────────────────── */

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);

    int choice;

    load_students();

    while (1)
    {
        printf("\n===== Attendance Manager =====\n");
        printf(" 1. Add Student\n");
        printf(" 2. Display Students\n");
        printf(" 3. Mark Attendance\n");
        printf(" 4. Show Attendance Percentage\n");
        printf(" 5. Sort Students by Attendance\n");
        printf(" 6. List Shortage / At Risk / Safe\n");
        printf(" 7. Generate Analysis Report\n");
        printf(" 8. Show Day-wise Attendance\n");
        printf(" 9. View Student Attendance History\n");
        printf("10. Modify Student\n");
        printf("11. Modify Attendance\n");
        printf("12. Delete Student\n");
        printf("13. Exit\n");
        printf("Enter Choice: ");
        fflush(stdout);

        choice = read_menu_choice();

        if (choice == -1)
            continue;

        switch (choice)
        {
            case 1:  add_student();                      break;
            case 2:  display_students();                 break;
            case 3:  mark_attendance();                  break;
            case 4:  show_all_percentages();             break;
            case 5:  sort_by_percentage();               break;
            case 6:  list_by_category();                 break;
            case 7:  generate_analysis_report();         break;
            case 8:  show_day_wise_attendance();          break;
            case 9:  view_student_attendance_history();   break;
            case 10: modify_student();                   break;
            case 11: modify_attendance();                break;
            case 12: delete_student();                   break;
            case 13:
            {
                char confirm;

                printf("Are you sure you want to exit? (y/n): ");
                fflush(stdout);
                scanf(" %c", &confirm);

                if (confirm == 'y' || confirm == 'Y')
                {
                    save_students();
                    printf("Exiting Program...\n");
                    fflush(stdout);
                    exit(0);
                }
                break;
            }
            default:
                printf("Invalid Choice.\n");
                fflush(stdout);
        }
    }

    return 0;
}