from calcul import *

cz = Matrice([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, -1]])
swap = Matrice([[1, 0, 0, 0], [0, 0, 1, 0], [0, 1, 0, 0], [0, 0, 0, 1]])
v = Matrice([[i] for i in [0, 0, 0, 0, 0, 0, 0, 1]])  # |111>
I = Matrice.identite(2)
# We want to apply cz between qubits 0 and 2
res = (swap @ I) * (I @ cz) * (swap @ I) * v
print(res)
