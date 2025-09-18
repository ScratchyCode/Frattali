from PIL import Image

def triangolo_tartaglia_txt(n, nome_file="triangolo.txt"):
    with open(nome_file, "w") as f:
        for i in range(n):
            riga = []
            for j in range(i + 1):
                # calcolo combinatorio classico
                riga.append(str(coeff_binomiale(i, j)))
            f.write(" ".join(riga) + "\n")
    print(f"Triangolo salvato in {nome_file}")

def coeff_binomiale(n, k):
    from math import comb
    return comb(n, k)

def genera_frattale_tartaglia(n, scala=1, nome_file="tartaglia.png"):
    larghezza = 2 * n * scala
    altezza = n * scala

    # modalità "1" = immagine binaria (pixel 0=nero, 1=bianco)
    img = Image.new("1", (larghezza, altezza), color=0)
    px = img.load()

    for i in range(n):
        for j in range(i + 1):
            if (i & j) == j:  # coefficiente dispari -> pixel bianco
                x = larghezza // 2 + int((j - i / 2) * scala)
                y = i * scala
                for dx in range(scala):
                    for dy in range(scala):
                        if 0 <= x + dx < larghezza and 0 <= y + dy < altezza:
                            px[x + dx, y + dy] = 1

    img.save(nome_file)
    print(f"Immagine salvata come {nome_file}")

if __name__ == "__main__":
    N = int(input("Numero righe nel triangolo di Tartaglia: "))

    # se N è piccolo salvo anche il txt
    if N <= 200:
        triangolo_tartaglia_txt(N, "triangolo.txt")
    else:
        print("N troppo grande: salto la generazione del file txt.")

    # genero sempre l'immagine
    genera_frattale_tartaglia(N, scala=1, nome_file="tartaglia.png")

