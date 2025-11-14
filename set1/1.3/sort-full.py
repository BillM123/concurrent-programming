def sort_file_halves_and_full(filename):
    with open(filename, 'r') as file:
        numbers = [int(line.strip()) for line in file if line.strip().isdigit()]

    midpoint = len(numbers) // 2

    first_half_sorted = sorted(numbers[:midpoint])
    second_half_sorted = sorted(numbers[midpoint:])

    sorted_halves = first_half_sorted + second_half_sorted
    fully_sorted = sorted(numbers)

    with open('sorted_halves_output.txt', 'w') as halves_file:
        for number in sorted_halves:
            halves_file.write(f"{number}\n")

    with open('fully_sorted_output.txt', 'w') as full_file:
        for number in fully_sorted:
            full_file.write(f"{number}\n")


input_filename = 'filePreSorted.txt'
sort_file_halves_and_full(input_filename)
