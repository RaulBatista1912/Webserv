#!/usr/bin/env python3

import os
import sys
from urllib.parse import unquote

# Bubble sort
def bubble_sort(arr):
    n = len(arr)
    for i in range(n):
        for j in range(0, n - i - 1):
            if arr[j] > arr[j + 1]:
                arr[j], arr[j + 1] = arr[j + 1], arr[j]
    return arr

# Récupération des paramètres GET
query = os.environ.get("QUERY_STRING", "")

params = {}
for pair in query.split("&"):
    if "=" in pair:
        key, value = pair.split("=", 1)
        params[key] = unquote(value)

# Header obligatoire CGI (note: sys.stdout.write pour contrôler exactement les \r\n)
sys.stdout.write("Content-Type: text/plain\r\n\r\n")

if "numbers" in params:
    try:
        numbers = list(map(int, params["numbers"].split(",")))
        sorted_numbers = bubble_sort(numbers)

        print(f"sorted: {sorted_numbers}")

    except:
        print("error: invalid input")
else:
    print("usage: ?numbers=5,2,9,1")