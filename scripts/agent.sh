source ./.env
read -p "Are you sure you want to continue? (y/n): " confirm
if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
  echo "Aborted."
  exit 1
fi
./build/agent ./test.html --key $API_KEY --model gpt-5-mini --prompt_id pmpt_68dab642df5c8193b611a4b526c3661d050b6e8ebdd8c5c0
