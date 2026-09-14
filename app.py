import customtkinter as ctk
from tkinter import filedialog, messagebox
from pathlib import Path
import subprocess
import threading


BASE_DIR = Path(__file__).resolve().parent
DATASETS_DIR = BASE_DIR / "datasets"
GREEDY_DIR = BASE_DIR / "main.exe"


class App(ctk.CTk):

    def __init__(self):
        super().__init__()

        self.title("Generator rasporeda")
        self.geometry("600x400")

        self.selected_dataset = None

        self.title_label = ctk.CTkLabel(
            self,
            text="Generator početnog rasporeda",
            font=ctk.CTkFont(size=24, weight="bold")
        )
        self.title_label.pack(pady=(30, 20))

        # Gumb za odabir dataseta
        self.dataset_button = ctk.CTkButton(
            self,
            text="Odaberi dataset",
            command=self.select_dataset
        )
        self.dataset_button.pack(pady=10)

        # Prikaz trenutno odabranog dataseta
        self.dataset_label = ctk.CTkLabel(
            self,
            text="Dataset nije odabran"
        )
        self.dataset_label.pack(pady=10)

        # Generiranje rasporeda
        self.generate_button = ctk.CTkButton(
            self,
            text="Generiraj početni raspored",
            command=self.generate_schedule
        )
        self.generate_button.pack(pady=20)

        self.status_label = ctk.CTkLabel(
            self,
            text=""
        )
        self.status_label.pack(pady=10)


    def select_dataset(self):
        file_path = filedialog.askopenfilename(
            title="Odaberi dataset",
            initialdir=DATASETS_DIR,
            filetypes=[
                ("TIM files", "*.tim"),
                ("All files", "*.*")
            ]
        )

        if file_path:
            self.selected_dataset = Path(file_path)

            self.dataset_label.configure(
                text=f"Odabrano: {self.selected_dataset.name}"
            )


    def generate_schedule(self):
        if self.selected_dataset is None:
            messagebox.showwarning(
                "Dataset nije odabran",
                "Prvo odaberi dataset."
            )
            return

        self.generate_button.configure(state="disabled")
        self.status_label.configure(text="Generiranje rasporeda...")

        # Make se izvršava u drugom threadu
        # kako se GUI ne bi zamrznuo
        thread = threading.Thread(
            target=self.run_greedy,
            daemon=True
        )
        thread.start()


    def run_greedy(self):
        try:
            result = subprocess.run(
                [
                    "make",
                    "run",
                    f"DATASET={self.selected_dataset}"
                ],
                cwd=GREEDY_DIR,
                capture_output=True,
                text=True
            )

            self.after(
                0,
                self.generation_finished,
                result
            )

        except Exception as e:
            self.after(
                0,
                self.generation_error,
                str(e)
            )


    def generation_finished(self, result):
        self.generate_button.configure(state="normal")

        if result.returncode == 0:
            self.status_label.configure(
                text="Početni raspored uspješno generiran."
            )

            print(result.stdout)

        else:
            self.status_label.configure(
                text="Greška pri generiranju rasporeda."
            )

            print(result.stderr)


    def generation_error(self, error):
        self.generate_button.configure(state="normal")

        self.status_label.configure(
            text="Greška pri pokretanju programa."
        )

        messagebox.showerror(
            "Greška",
            error
        )


if __name__ == "__main__":
    app = App()
    app.mainloop()