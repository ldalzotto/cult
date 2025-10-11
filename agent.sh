source ./.env
read -p "Are you sure you want to continue? (y/n): " confirm
if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
  echo "Aborted."
  exit 1
fi
./build/agent ./test.html --key $API_KEY --model gpt-5-mini
