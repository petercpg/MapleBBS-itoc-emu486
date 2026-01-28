# 砍掉吃資源 > 30% 的 process
# Use ps -ax -o user,pid,pcpu (compatible with Linux/FreeBSD)
# awk checks:
# $1 (user) == "bbs" (Safety check: only kill bbs user processes)
# $3 (pcpu) > 30.0
ps -ax -o user,pid,pcpu | awk '$1 == "bbs" && $3 > 30.0 {print $2}' | xargs kill -9 2>/dev/null
