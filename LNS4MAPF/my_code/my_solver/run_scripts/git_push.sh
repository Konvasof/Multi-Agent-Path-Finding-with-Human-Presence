#!/bin/bash

if [ -z "$1" ]; then
    echo "Chyba: Nezadala jsi commit zprávu!"
    echo "Použití: ./git_push.sh \"Tvoje zpráva o úpravě kódu\""
    exit 1
fi

echo "Přidávám změny (git add .)..."
git add .

echo "Vytvářím commit (git commit -m \"$1\")..."
git commit -m "$1"

echo "Odesílám na server (git push)..."
git push

echo "Hotovo! Změny jsou v repozitáři."