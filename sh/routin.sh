#!/bin/sh
####################################################################
# 看板排行榜統計
basedir=/home/bbs/manage
logfile=/home/bbs/gem/@/@hotboard
hotweek=/home/bbs/gem/@/@hotweek
hotmonth=/home/bbs/gem/@/@hotmonth

# Portable date handling for Linux (GNU) and FreeBSD (BSD)
OS=`uname -s`
if [ "$OS" = "FreeBSD" ]; then
    yestarday=`date -v-1d +%y%m%d`
    today=`date +%y%m%d`
else
    # Assume GNU date (Linux)
    yestarday=`date --date='1 day ago' +%y%m%d`
    today=`date +%y%m%d`
fi

if [ ! -d "$basedir/brd" ]; then
    mkdir -p "$basedir/brd"
fi

if [ -f /home/bbs/run/brd_usies.log ]; then
    cp /home/bbs/run/brd_usies.log "$basedir/brd/$yestarday.log"
    cat /home/bbs/run/brd_usies.log | awk '{print $1}' | uniq -c | awk '{printf "%-15s%s\n" ,$2,$1}' | sort -rn -k 2 > "$basedir/board.log"
else
    touch "$basedir/board.log"
fi

echo "看板名稱 參觀人次" | awk '{printf("%-15s%s\n",$1,$2)}' > "$logfile"
echo "名次 看板名稱 參觀人次" | awk '{printf("%-8s%-15s%s\n",$1,$2,$3)}' > "$hotweek"
echo "名次 看板名稱 參觀人次" | awk '{printf("%-8s%-15s%s\n",$1,$2,$3)}' > "$hotmonth"
echo "-----------------------------" >> "$logfile"
echo "-----------------------------" >> "$hotweek"
echo "-----------------------------" >> "$hotmonth"

# List directory contents safely. On some systems ls -l adds extra info, simpler to use echo or find,
# but maintaining original logic with portable tweaks.
# "ls -F ... | grep /" identifies directories reliably on standard shells.
ls -F /home/bbs/brd | grep "/" | sed 's/\///' > "$basedir/boardlist"

echo "目前看板個數:`wc -l < $basedir/boardlist`" >> "$logfile"
if [ -f "$basedir/board.log" ]; then
    cat "$basedir/board.log" >> "$logfile"
fi

# find usage is generally portable for -mtime.
# Explicitly handling spaces in filenames is good practice, though unlikely for BBS boards.
find "$basedir/brd" -mtime -7 -type f -exec cat '{}' \; | awk '{print $1}' | sort | uniq -c | awk '{printf "%-15s%s\n" ,$2,$1}' | sort -rn -k 2 > "$basedir/week"
cat -b "$basedir/week" >> "$hotweek"

find "$basedir/brd" -mtime -30 -type f -exec cat '{}' \; | awk '{print $1}' | sort | uniq -c | awk '{printf "%-15s%s\n" ,$2,$1}' | sort -rn -k 2 > "$basedir/month"
cat -b "$basedir/month" >> "$hotmonth"

# Use while read loop which is cleaner than for loop over cat output
while read board
do
    judge=`grep "^$board" "$basedir/board.log"`
    if [ -z "$judge" ]; then
        echo "$board 0" | awk '{printf("%-15s%s\n",$1,$2)}' >> "$logfile"
    fi
    judge=`grep "^$board" "$basedir/week"`
    if [ -z "$judge" ]; then
        echo "$board 0" | awk '{printf("%-15s%s\n",$1,$2)}' >> "$hotweek"
    fi
    judge=`grep "^$board" "$basedir/month"`
    if [ -z "$judge" ]; then
        echo "$board 0" | awk '{printf("%-15s%s\n",$1,$2)}' >> "$hotmonth"
    fi
done < "$basedir/boardlist"
