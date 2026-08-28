url="$1"
out="$2"
mkdir -p "$(dirname "$out")"
for attempt in $(seq 1 10); do
    curl -L -C - --retry 5 --retry-delay 10 -o "$out" "$url" && break
    sleep 5
done
