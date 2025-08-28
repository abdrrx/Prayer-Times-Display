#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// --- data we receive from PC ---
struct Prayer {
  const char* name;
  int minutes; // minutes since midnight local
} prayers[6] = {
  {"Fajr",    -1},
  {"Sunrise", -1},
  {"Dhuhr",   -1},
  {"Asr",     -1},
  {"Maghrib", -1},
  {"Isha",    -1}
};

String inLine = "";
int curH=0, curM=0, curS=0;
bool haveTimes=false;

// --- helpers ---
int toMinutes(const String& hhmm){
  // expects "HH:MM"
  if (hhmm.length()<4) return -1;
  int p = hhmm.indexOf(':'); if (p<0) return -1;
  int h = hhmm.substring(0,p).toInt();
  int m = hhmm.substring(p+1).toInt();
  if(h<0||h>23||m<0||m>59) return -1;
  return h*60+m;
}

String two(int v){ char b[3]; snprintf(b, sizeof(b), "%02d", v); return String(b); }
String fit16(const String &s){ if((int)s.length()>=16) return s.substring(0,16); String o=s; while((int)o.length()<16)o+=" "; return o; }

void parsePrayers(const String& line){
  // Format: PRAYERS|Fajr|HH:MM|Sunrise|HH:MM|Dhuhr|HH:MM|Asr|HH:MM|Maghrib|HH:MM|Isha|HH:MM
  int idx=0, start=0;
  String parts[13]; // "PRAYERS" + 12 items
  while (true){
    int bar = line.indexOf('|', start);
    if (bar<0){ parts[idx++] = line.substring(start); break; }
    parts[idx++] = line.substring(start, bar);
    start = bar+1;
    if (idx>=13) break;
  }
  if (idx<13 || parts[0]!="PRAYERS") return;

  // Map pairs
  for(int i=0;i<6;i++){
    String name = parts[1 + i*2];
    String time = parts[2 + i*2];
    int mins = toMinutes(time);
    prayers[i].minutes = mins;
    // keep names as defaults; (API names match our order)
  }
  haveTimes = true;
}

void parseTime(const String& line){
  // Format: TIME|HH:MM:SS
  int p1 = line.indexOf('|');
  if(p1<0) return;
  String t = line.substring(p1+1);
  int p2 = t.indexOf(':'), p3 = t.indexOf(':', p2+1);
  if(p2<0||p3<0) return;
  curH = t.substring(0,p2).toInt();
  curM = t.substring(p2+1,p3).toInt();
  curS = t.substring(p3+1).toInt();
}

int minutesNow(){ return curH*60 + curM; }

int nextPrayerIndex(){
  int now = minutesNow();
  int best=-1, bestDelta=24*60+1;
  for(int i=0;i<6;i++){
    int t = prayers[i].minutes;
    if(t<0) continue;
    int d = t - now;
    if(d<0) d += 24*60; // wrap to next day
    if(d<bestDelta){ bestDelta=d; best=i; }
  }
  return best;
}

void formatCountdown(int minutesLeft, int &hh, int &mm){
  hh = minutesLeft/60;
  mm = minutesLeft%60;
}

void setup(){
  lcd.begin(16,2);
  lcd.clear(); lcd.print("Prayer Times");
  Serial.begin(9600);
}

unsigned long lastRefresh=0;

void loop(){
  // --- read serial lines ---
  while(Serial.available()){
    char c = Serial.read();
    if(c=='\n'){
      if(inLine.startsWith("PRAYERS|"))      parsePrayers(inLine);
      else if(inLine.startsWith("TIME|"))     parseTime(inLine);
      inLine = "";
    } else if(c!='\r'){
      if(c>=32 && c<=126) inLine += c; // keep ASCII
    }
  }

  // --- update LCD at ~4 Hz for smooth clock ---
  if(millis()-lastRefresh >= 250){
    lastRefresh = millis();

    // Line 1: current time
    String clock = two(curH)+":"+two(curM)+":"+two(curS);
    lcd.setCursor(0,0); lcd.print(fit16(clock + "          ")); // pad/clear

    // Line 2: next prayer countdown
    if(haveTimes){
      int nxt = nextPrayerIndex();
      if(nxt>=0){
        int nowM = minutesNow();
        int target = prayers[nxt].minutes;
        int delta = target - nowM; if(delta<0) delta += 24*60;
        // subtract passing seconds to tighten countdown feel
        int showDelta = (delta*60) - curS; if(showDelta<0) showDelta=0;
        int hh, mm; formatCountdown(showDelta/60, hh, mm);
        String when = two(target/60)+":"+two(target%60);
        String line2 = String(prayers[nxt].name) + " " + when + " T-" + two(hh)+":"+two(mm);
        lcd.setCursor(0,1); lcd.print(fit16(line2));
      } else {
        lcd.setCursor(0,1); lcd.print(fit16("Waiting times..."));
      }
    } else {
      lcd.setCursor(0,1); lcd.print(fit16("Awaiting PC data"));
    }
  }
}
