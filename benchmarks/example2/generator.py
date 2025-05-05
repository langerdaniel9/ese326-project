import random


def generate_benchmark(num_nodes=1000, num_terminals=50, num_nets=800, output_dir="benchmarks/example2/"):
    # Generate nodes file
    with open(f"{output_dir}example2.nodes", "w") as f:
        f.write("UCLA nodes 1.0\n")
        f.write("# Created  :  May 05 2025\n")
        f.write("# User     :  Example User\n\n")
        f.write(f"NumNodes      :  {num_nodes}\n")
        f.write(f"NumTerminals  :  {num_terminals}\n\n")

        # Select random nodes to be terminals
        terminals = set(random.sample(range(num_nodes), num_terminals))

        for i in range(num_nodes):
            width = random.randint(10, 25)
            height = random.randint(10, 25)
            terminal_str = "    terminal" if i in terminals else ""
            f.write(f"    n{i}     {width}     {height}{terminal_str}\n")

    # Generate nets file
    with open(f"{output_dir}example2.nets", "w") as f:
        f.write("UCLA nets 1.0\n")
        f.write("# Created  :  May 05 2025\n")
        f.write("# User     :  Example User\n\n")
        f.write(f"NumNets   :  {num_nets}\n")

        for i in range(num_nets):
            # Random degree between 3 and 15
            degree = random.randint(3, 15)
            f.write(f"NetDegree :  {degree}    net{i}\n")

            # Select random nodes for this net
            net_nodes = random.sample(range(num_nodes), degree)

            # First node is input, others are output
            for j, node in enumerate(net_nodes):
                direction = "I" if j == 0 else "O"
                x, y = float(j), float(j)
                f.write(f"        n{node}   {direction}  :   {x:.1f}  {y:.1f}\n")


# Run the generator
generate_benchmark()
