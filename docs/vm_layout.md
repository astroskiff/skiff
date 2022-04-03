# VM Layout

*Notes*

Libraries should be loaded into seperate instances of virtual machines that share a channel with the main executable. 
This will enable easy parallelism and a simple memory layout for constants. 