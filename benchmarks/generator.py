import random
import os
import time


def generate_benchmark(size="medium", base_dir="benchmarks", custom_name=None):
    """
    Generate benchmark files of different sizes.

    Parameters:
    - size: "small" (1k nodes), "medium" (10k nodes), "large" (50k nodes), "xlarge" (100k nodes)
    - base_dir: Base directory for benchmarks
    - custom_name: Custom name for output files (default uses size as prefix)
    """
    # Define sizes
    sizes = {
        "small": {"nodes": 1000, "terminals": 50, "nets": 800},
        "medium": {"nodes": 10000, "terminals": 500, "nets": 8000},
        "large": {"nodes": 50000, "terminals": 2500, "nets": 40000},
        "xlarge": {"nodes": 100000, "terminals": 5000, "nets": 80000},
        "xxlarge": {"nodes": 250000, "terminals": 12500, "nets": 200000},
    }

    if size not in sizes:
        print(f"Invalid size '{size}'. Using 'medium' instead.")
        size = "medium"

    num_nodes = sizes[size]["nodes"]
    num_terminals = sizes[size]["terminals"]
    num_nets = sizes[size]["nets"]

    # Create output filename and directory structure
    basename = custom_name if custom_name else f"example_{size}"

    # Create directory structure: /benchmarks/NAME/
    output_dir = os.path.join(base_dir, basename)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # File paths will be /benchmarks/NAME/NAME.ext
    file_prefix = os.path.join(output_dir, basename)

    print(f"Generating benchmark with {num_nodes} nodes, {num_terminals} terminals, and {num_nets} nets...")
    start_time = time.time()

    # Generate nodes file
    print("Generating nodes file...")
    with open(f"{file_prefix}.nodes", "w") as f:
        f.write("UCLA nodes 1.0\n")
        f.write(f"# Created  :  {time.strftime('%b %d %Y')}\n")
        f.write("# User     :  Example User\n\n")
        f.write(f"NumNodes      :  {num_nodes}\n")
        f.write(f"NumTerminals  :  {num_terminals}\n\n")

        # Select random nodes to be terminals
        terminals = set(random.sample(range(num_nodes), num_terminals))

        # Write nodes in batches to save memory
        batch_size = 10000
        for i in range(0, num_nodes, batch_size):
            batch_end = min(i + batch_size, num_nodes)
            node_lines = []

            for j in range(i, batch_end):
                width = random.randint(10, 25)
                height = random.randint(10, 25)
                terminal_str = "    terminal" if j in terminals else ""
                node_lines.append(f"    n{j}     {width}     {height}{terminal_str}\n")

            f.writelines(node_lines)
            if num_nodes > 10000:
                print(f"  Processed {batch_end}/{num_nodes} nodes ({int(batch_end/num_nodes*100)}%)")

    # Generate nets file
    print("Generating nets file...")
    with open(f"{file_prefix}.nets", "w") as f:
        f.write("UCLA nets 1.0\n")
        f.write(f"# Created  :  {time.strftime('%b %d %Y')}\n")
        f.write("# User     :  Example User\n\n")
        f.write(f"NumNets   :  {num_nets}\n")

        # Write nets in batches to save memory
        batch_size = 5000
        for i in range(0, num_nets, batch_size):
            batch_end = min(i + batch_size, num_nets)
            net_lines = []

            for j in range(i, batch_end):
                # Random degree between 3 and 15
                degree = random.randint(3, 15)
                net_lines.append(f"NetDegree :  {degree}    net{j}\n")

                # Select random nodes for this net
                net_nodes = random.sample(range(num_nodes), degree)

                # First node is input, others are output
                for k, node in enumerate(net_nodes):
                    direction = "I" if k == 0 else "O"
                    x, y = float(k), float(k)
                    net_lines.append(f"        n{node}   {direction}  :   {x:.1f}  {y:.1f}\n")

            f.writelines(net_lines)
            if num_nets > 5000:
                print(f"  Processed {batch_end}/{num_nets} nets ({int(batch_end/num_nets*100)}%)")

    # Create aux file
    with open(f"{file_prefix}.aux", "w") as f:
        f.write(
            f"RowBasedPlacement :  {basename}.nodes  {basename}.nets  {basename}.wts  {basename}.pl  {basename}.scl  {basename}.shapes  {basename}.route\n"
        )

    elapsed_time = time.time() - start_time
    print(f"Benchmark generation complete in {elapsed_time:.1f} seconds.")
    print(f"Files created in directory {output_dir}/")


def generate_all_benchmarks(base_dir="benchmarks"):
    """Generate all benchmark sizes at once"""
    print("Generating all benchmark sizes...")

    # Make sure base directory exists
    if not os.path.exists(base_dir):
        os.makedirs(base_dir)

    # Generate all sizes
    for size in ["small", "medium", "large", "xlarge"]:
        print(f"\n=== Generating {size.upper()} benchmark ===")
        generate_benchmark(size, base_dir)

    print("\nAll benchmarks generated successfully!")


if __name__ == "__main__":
    # To generate a specific size:
    # generate_benchmark("small")  # Creates /benchmarks/example_small/example_small.*

    # To generate all sizes:
    generate_all_benchmarks()  # Creates all benchmark sizes in proper directories
