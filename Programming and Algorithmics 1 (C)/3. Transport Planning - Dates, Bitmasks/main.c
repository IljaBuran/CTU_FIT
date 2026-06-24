#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

constexpr unsigned DOW_MON      = 0b0000'0001;
constexpr unsigned DOW_TUE      = 0b0000'0010;
constexpr unsigned DOW_WED      = 0b0000'0100;
constexpr unsigned DOW_THU      = 0b0000'1000;
constexpr unsigned DOW_FRI      = 0b0001'0000;
constexpr unsigned DOW_SAT      = 0b0010'0000;
constexpr unsigned DOW_SUN      = 0b0100'0000;
constexpr unsigned DOW_WORKDAYS = DOW_MON | DOW_TUE | DOW_WED | DOW_THU | DOW_FRI;
constexpr unsigned DOW_WEEKEND  = DOW_SAT | DOW_SUN;
constexpr unsigned DOW_ALL      = DOW_WORKDAYS | DOW_WEEKEND;

typedef struct TDate
{
  unsigned m_Year;
  unsigned m_Month;
  unsigned m_Day;
} TDATE;

TDATE makeDate ( unsigned y,
                 unsigned m,
                 unsigned d )
{
  TDATE res = { y, m, d };
  return res;
}
#endif /* __PROGTEST__ */
#include <stdbool.h>

#define YEAR_MIN 2000
#define YEAR_MAX 1000000000

/* Function to roundup the number */
#define DivisionRoundup(a,b) (((a) + (b) - 1) / (b))

/* Technically should be signed, but we are working with 2000 <= year <= 1000000000 */
bool IsLeapYear(unsigned year) { return ((year % 4 == 0) && ((year % 100 != 0 && year % 4000 != 0) || (year % 100 != 0 && year % 400 == 0) || (year % 4000 != 0 && year % 400 == 0))); }

/* Returns array with number of days in months */
/* If year is leap, then february is 29 days */
unsigned DaysInMonth(unsigned year, unsigned month)
{
  unsigned arr[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (IsLeapYear(year))
    arr[2] = 29;
  return arr[month];
}

/* Sakamoto's Algorithm */
int DetermineDay(TDATE date){ return (date.m_Day += date.m_Month < 3 ? date.m_Year-- : date.m_Year - 2, 23 * date.m_Month / 9 + date.m_Day + 4 + date.m_Year / 4 - date.m_Year / 100 + date.m_Year / 400 - date.m_Year / 4000) % 7; }


/* Checks whether input date is valid */
/* YEAR_MAX = 1000000000, YEAR_MIN = 2000 */
bool IsDateValid(TDATE date)
{
  if (date.m_Year >= YEAR_MIN && date.m_Year <= YEAR_MAX)
    if (date.m_Month >= 1 && date.m_Month <= 12)
      if (date.m_Day >= 1 && date.m_Day <= DaysInMonth(date.m_Year, date.m_Month))
        return true;
  return false;
}

bool IsDateOrderValid(TDATE d1, TDATE d2) {
    if (d1.m_Year != d2.m_Year)
        return d1.m_Year < d2.m_Year;
    if (d1.m_Month != d2.m_Month)
        return d1.m_Month < d2.m_Month;
    return d1.m_Day <= d2.m_Day;
}

bool containsPattern(unsigned mask, unsigned day) { return (mask & day) == day; }

/* Assign values depending on dowMask */
/* {empty, monday, tuesday, wednesday, thursday, friday, saturday, sunday} */
/* {empty, perWorkDay, perWorkDay, perWorkDay, perWorkDay, perWorkDay, perWorkDay/2, perWorkDay/3} */
void AssignValuesToArr(unsigned dowMask, unsigned perWorkDay, unsigned week[8])
{
  unsigned day = DOW_MON;
  for (size_t i = 1; i <= 7; i++)
  {
    if (containsPattern(dowMask, day))
    {  
      if(i == 6)
        week[i] = DivisionRoundup(perWorkDay, 2); 
      else if(i == 7)  
        week[i] = DivisionRoundup(perWorkDay, 3);
      else
        week[i] = perWorkDay;
    }
    else week[i] = 0;
    day <<= 1;
  }
}

#ifdef DEBUG
void PrintDates(TDATE d1, TDATE d2)
{
  printf("Dates: %u/%u/%u -> %u/%u/%u\n", d1.m_Day, d1.m_Month, d1.m_Year, d2.m_Day, d2.m_Month, d2.m_Year);
}
#endif

long long countConnections (TDATE from, TDATE to, unsigned perWorkDay, unsigned dowMask)
{
  /* DEBUG prints */
  #ifdef DEBUG
    PrintDates(from, to);
    printf("perWorkDay: %u\n", perWorkDay);
    printf("dowMask: %u\n", dowMask);
  #endif
  /* Data validity check */
  if (!IsDateValid(from) || !IsDateValid(to) || !IsDateOrderValid(from, to)) return -1;
  else if (perWorkDay == 0 || dowMask == 0) return 0;

  long long connections = 0; // result to be returned 
  /* Array contains count per day */
  unsigned weekarr[8]; AssignValuesToArr(dowMask, perWorkDay, weekarr);
  
  /* DEBUG prints */
  #ifdef DEBUG
  printf("Week Array: ");
  for(int i = 1; i <= 7; i++)
  { 
    if (i == 1)
      printf("(");
    printf("%u", weekarr[i]);
    if (i == 7) printf(")\n");
    else printf(",");
  }
  unsigned totalDays = 0;
  #endif 

  /* bool done -> variable to tracked whether algorithm has reached end date*/
  /* bool checked -> variable to tracked whether reset from 1 day/month the next iteration */
  /* assigning start values to variables to track day algorithm is currenly at */
  bool done = false, checked = false; 
  unsigned currentYear = from.m_Year, currentMonth = from.m_Month, currentDay = from.m_Day;
  unsigned weekday = DetermineDay(from); /*  */ 
  if (weekday == 0) weekday = 7;

  /* Naive algorithm, going through every day, until last date */
  for (currentYear = from.m_Year; currentYear <= to.m_Year && !done; currentYear++)
  {
    if (checked) currentMonth = 1;
    for ( ; currentMonth <= 12 && !done; currentMonth++)
    {
      if (checked) currentDay = 1;
      else checked = true;
      for ( ; currentDay <= DaysInMonth(currentYear, currentMonth); currentDay++)
      {
        connections += weekarr[weekday];
        
        #ifdef DEBUG
        printf("connections += %u\n", weekarr[weekday]);
        totalDays++;
        #endif 

        if (currentYear == to.m_Year && currentMonth == to.m_Month && currentDay == to.m_Day) { done = true; break; }
        if (weekday == 7) weekday = 1;
        else weekday++;
      }
    }
  }

  #ifdef DEBUG
  printf("Total days: %u\n", totalDays);
  printf("Connections: %lld\n", connections);
  #endif 

  return connections;
}
TDATE endDate (TDATE from, long long connections, unsigned perWorkDay, unsigned dowMask)
{
  /* Check validity of inputs */
  if (!IsDateValid(from) || perWorkDay == 0 || dowMask == 0) return makeDate(0000, 00, 00);
  
  /* long long remaining tracks how many connections are left */
  long long remaining = connections;
  
  /* Array contains count per day */
  unsigned weekarr[8]; AssignValuesToArr(dowMask, perWorkDay, weekarr);
  
  /* bool done -> variable to tracked whether algorithm has reached end date*/
  /* bool checked -> variable to tracked whether reset from 1 day/month the next iteration */
  bool done = false, checked = false; 
  
  /* DEBUG prints */
  #ifdef DEBUG
  printf("Week Array: ");
  for(int i = 1; i <= 7; i++)
  { 
    if (i == 1)
      printf("(");
    printf("%u", weekarr[i]);
    if (i == 7) printf(")\n");
    else printf(",");
  }
  unsigned totalDays = 0;
  #endif 

  /* assigning start values to variables to track day algorithm is currenly at */
  unsigned currentYear = from.m_Year, currentMonth = from.m_Month, currentDay = from.m_Day;
  
  /* Assigns weekday at beginning  */ 
  unsigned weekday = DetermineDay(from); 
  if (weekday == 0) weekday = 7;
  if (connections < weekarr[weekday])
    return makeDate(0000, 00, 00);
  /* Naive algorithm, going through every day, until perWorkDay of currentDay > remaining */
  for (currentYear = from.m_Year; !done; currentYear++)
  {
    if (checked) currentMonth = 1;
    for ( ; currentMonth <= 12 && !done; currentMonth++)
    {
      if (checked) currentDay = 1;
      else checked = true;
      for ( ; currentDay <= DaysInMonth(currentYear, currentMonth); currentDay++)
      {
        #ifdef DEBUG
        printf("%u/%u/%u\n", currentDay, currentMonth, currentYear);
        printf("current remaining: %lld\n", remaining);
        printf("remaining -= %u\n", weekarr[weekday]);
        totalDays++;
        #endif 
        
        remaining -= weekarr[weekday];
        
        if (weekday == 7) weekday = 1;
        else weekday++;
        
        #ifdef DEBUG
        printf("%u > %lld\n",weekarr[weekday], remaining);
        #endif 

        if (weekarr[weekday] > remaining){ done = true; break; }
      }
    }
  }
    /* DEBUG prints */
  #ifdef DEBUG
  printf("Remaining: %lld\n", remaining);
  printf("Total days: %u\n", totalDays);
  #endif 
  return makeDate(currentYear - 1, currentMonth - 1, currentDay);
}


#ifndef __PROGTEST__
int main ()
{ 
  TDATE d;
  
  assert ( countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 31 ), 1, DOW_ALL ) == 31 );
  assert ( countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 31 ), 10, DOW_ALL ) == 266 );
  assert ( countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 31 ), 1, DOW_WED ) == 5 );
  assert ( countConnections ( makeDate ( 2024, 10, 2 ), makeDate ( 2024, 10, 30 ), 1, DOW_WED ) == 5 );
  assert ( countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 1 ), 10, DOW_TUE ) == 10 );
  assert ( countConnections ( makeDate ( 2024, 10, 1 ), makeDate ( 2024, 10, 1 ), 10, DOW_WED ) == 0 );
  assert ( countConnections ( makeDate ( 2024, 1, 1 ), makeDate ( 2034, 12, 31 ), 5, DOW_MON | DOW_FRI | DOW_SAT ) == 7462 );
  assert ( countConnections ( makeDate ( 2024, 1, 1 ), makeDate ( 2034, 12, 31 ), 0, DOW_MON | DOW_FRI | DOW_SAT ) == 0 );
  assert ( countConnections ( makeDate ( 2024, 1, 1 ), makeDate ( 2034, 12, 31 ), 100, 0 ) == 0 );
  assert ( countConnections ( makeDate ( 2024, 10, 10 ), makeDate ( 2024, 10, 9 ), 1, DOW_MON ) == -1 );
  assert ( countConnections ( makeDate ( 2024, 2, 29 ), makeDate ( 2024, 2, 29 ), 1, DOW_ALL ) == 1 );
  assert ( countConnections ( makeDate ( 2023, 2, 29 ), makeDate ( 2023, 2, 29 ), 1, DOW_ALL ) == -1 );
  assert ( countConnections ( makeDate ( 2100, 2, 29 ), makeDate ( 2100, 2, 29 ), 1, DOW_ALL ) == -1 );
  assert ( countConnections ( makeDate ( 2400, 2, 29 ), makeDate ( 2400, 2, 29 ), 1, DOW_ALL ) == 1 );
  assert ( countConnections ( makeDate ( 4000, 2, 29 ), makeDate ( 4000, 2, 29 ), 1, DOW_ALL ) == -1 );
  d = endDate ( makeDate ( 2024, 10, 1 ), 100, 1, DOW_ALL );
  assert ( d . m_Year == 2025 && d . m_Month == 1 && d . m_Day == 8 );
  d = endDate ( makeDate ( 2024, 10, 1 ), 100, 6, DOW_ALL );
  assert ( d . m_Year == 2024 && d . m_Month == 10 && d . m_Day == 20 );
  d = endDate ( makeDate ( 2024, 10, 1 ), 100, 1, DOW_WORKDAYS );
  assert ( d . m_Year == 2025 && d . m_Month == 2 && d . m_Day == 17 );
  d = endDate ( makeDate ( 2024, 10, 1 ), 100, 4, DOW_WORKDAYS );
  assert ( d . m_Year == 2024 && d . m_Month == 11 && d . m_Day == 4 );
  d = endDate ( makeDate ( 2024, 10, 1 ), 100, 1, DOW_THU );
  assert ( d . m_Year == 2026 && d . m_Month == 9 && d . m_Day == 2 );
  d = endDate ( makeDate ( 2024, 10, 1 ), 100, 2, DOW_THU );
  assert ( d . m_Year == 2025 && d . m_Month == 9 && d . m_Day == 17 );
  d = endDate ( makeDate ( 2024, 10, 1 ), 100, 0, DOW_THU );
  assert ( d . m_Year == 0 && d . m_Month == 0 && d . m_Day == 0 );
  d = endDate ( makeDate ( 2024, 10, 1 ), 100, 1, 0 );
  assert ( d . m_Year == 0 && d . m_Month == 0 && d . m_Day == 0 );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
