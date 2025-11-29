git diff --cached | xsel --clipboard --input
tmpfile=$(mktemp)

{
  echo "[USER]"
  echo
  xsel --clipboard --output
} > "$tmpfile"

echo "Pasted into: $tmpfile"

source ./.env
read -p "Are you sure you want to continue? (y/n): " confirm
if [[ "$confirm" != "y" && "$confirm" != "Y" ]]; then
  echo "Aborted."
  exit 1
fi
./build/agent $tmpfile --key $API_KEY --model gpt-5-mini --prompt_id pmpt_68db5ba146708195a344677ac7bfd80d059aa9a95733e34c

agent_text="$(sed -n '/\[AGENT\]/,$p' "$tmpfile" | sed '1d')"
msgfile=$(mktemp)
printf "%s" "$agent_text" > "$msgfile"

git commit -F "$msgfile"
# git commit -m "$agent_text"

rm $tmpfile
rm $msgfile