from itertools import product
names = "Static","Clip","Charecter","Rigid","Soft"
spacial = "Static", "Clip"


for i, (a, b) in enumerate(product(names, names)):
	# if a in spacial and b == a:
	# 	continue
	first_type = i % 5
	second_type = i // 5
	print(f"{a}{b}Collision = 0x{first_type | (second_type << 8):X},")
