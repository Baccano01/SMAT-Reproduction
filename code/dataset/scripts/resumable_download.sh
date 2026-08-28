url="$1"
out="$2"
mkdir -p "$(dirname "$out")"
for attempt in $(seq 1 10); do
    curl -L -C - --retry 5 --retry-delay 10 --connect-timeout 15 -o "$out" "$url" && exit 0 
    sleep 5
done

echo "FAILED: $url" >> failed.log
exit 1
