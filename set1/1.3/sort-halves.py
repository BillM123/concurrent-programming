def sort_file_halves(filename):
    with open(filename, 'r') as file:
        numbers = [int(line.strip()) for line in file if line.strip().isdigit()]

    midpoint = len(numbers) // 2

    # Sort the first and second halves separately
    first_half_sorted = sorted(numbers[:midpoint])
    second_half_sorted = sorted(numbers[midpoint:])

    # Combine the sorted halves
    sorted_numbers = first_half_sorted + second_half_sorted

    with open('sorted_halves_output.txt', 'w') as output_file:
        for number in sorted_numbers:
            output_file.write(f"{number}\n")

    print("The numbers have been sorted into 'sorted_halves_output.txt'")

input_filename = 'filePreSorted.txt'
sort_file_halves(input_filename)
