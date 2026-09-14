import customtkinter as ctk
from tkinter import filedialog, messagebox
from pathlib import Path
import re
import shutil
import subprocess
import threading


BASE_DIR = Path(__file__).resolve().parent

DATASETS_DIR = BASE_DIR / "datasets"
GREEDY_DIR = BASE_DIR / "greedy"
LOCAL_SEARCH_1_DIR = BASE_DIR / "local_search_method_1"

GREEDY_OUTPUT_DIR = BASE_DIR / "outputs"
LOCAL_SEARCH_1_OUTPUT_DIR = BASE_DIR / "ls1_outputs"

# LOCAL_SEARCH_2_DIR = BASE_DIR / "local_search_method_2"
# LOCAL_SEARCH_2_OUTPUT_DIR = BASE_DIR / "ls2_outputs"

NUMBER_OF_TIMESLOTS = 45
DAYS = ["Pon", "Uto", "Sri", "Čet", "Pet"]
SLOTS_PER_DAY = 9
ROOMS_PER_ROW = 4

class App(ctk.CTk):
    def __init__(self):
        super().__init__()

        ctk.set_appearance_mode("system")
        ctk.set_default_color_theme("blue")

        self.title("PECT - Generator rasporeda")
        self.geometry("1480x900")
        self.minsize(1100, 700)

        self.selected_dataset = None
        self.selected_dataset_number = None
        self.is_running = False

        # Omogućava da se glavni sadržaj širi s prozorom.
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(3, weight=1)

        self.create_header()
        self.create_dataset_section()
        self.create_action_buttons()
        self.create_results_section()

    # GUI
    def create_header(self):
        header = ctk.CTkFrame(self, corner_radius=0)
        header.grid(row=0, column=0, sticky="ew")
        header.grid_columnconfigure(0, weight=1)

        title = ctk.CTkLabel(
            header,
            text="Post-Enrolment Course Timetabling",
            font=ctk.CTkFont(size=28, weight="bold")
        )
        title.grid(row=0, column=0, padx=30, pady=(24, 24))

    def create_dataset_section(self):
        section = ctk.CTkFrame(self)
        section.grid(row=1, column=0, padx=24, pady=(18, 10), sticky="ew")
        section.grid_columnconfigure(1, weight=1)

        self.dataset_button = ctk.CTkButton(
            section,
            text="Odaberi dataset",
            width=180,
            height=40,
            command=self.select_dataset
        )
        self.dataset_button.grid(row=0, column=0, padx=(18, 12), pady=16)

        self.dataset_label = ctk.CTkLabel(
            section,
            text="Dataset nije odabran",
            anchor="w",
            font=ctk.CTkFont(size=14)
        )
        self.dataset_label.grid(row=0, column=1, padx=(0, 18), pady=16, sticky="ew")

    def create_action_buttons(self):
        section = ctk.CTkFrame(self)
        section.grid(row=2, column=0, padx=24, pady=10, sticky="ew")

        for column in range(3):
            section.grid_columnconfigure(column, weight=1)

        self.greedy_button = ctk.CTkButton(
            section,
            text="Greedy",
            height=44,
            command=self.start_greedy
        )
        self.greedy_button.grid(
            row=0, column=0, padx=(18, 8), pady=(16, 8), sticky="ew"
        )

        self.local_search_1_button = ctk.CTkButton(
            section,
            text="Local Search 1",
            height=44,
            command=self.start_local_search_1
        )
        self.local_search_1_button.grid(
            row=0, column=1, padx=8, pady=(16, 8), sticky="ew"
        )

        self.local_search_2_button = ctk.CTkButton(
            section,
            text="Local Search 2",
            height=44,
            state="disabled",
            command=self.start_local_search_2
        )
        self.local_search_2_button.grid(
            row=0, column=2, padx=(8, 18), pady=(16, 8), sticky="ew"
        )

        self.status_label = ctk.CTkLabel(
            section,
            text="Odaberi dataset za početak.",
            anchor="w"
        )
        self.status_label.grid(
            row=1, column=0, columnspan=3,
            padx=18, pady=(4, 14), sticky="ew"
        )

def create_results_section(self):
        results_container = ctk.CTkFrame(self)
        results_container.grid(
            row=3, column=0, padx=24, pady=(10, 24), sticky="nsew"
        )
        results_container.grid_columnconfigure(0, weight=1)
        results_container.grid_rowconfigure(1, weight=1)

        self.results_title = ctk.CTkLabel(
            results_container,
            text="Raspored",
            anchor="w",
            font=ctk.CTkFont(size=20, weight="bold")
        )
        self.results_title.grid(
            row=0, column=0, padx=18, pady=(16, 8), sticky="ew"
        )

        self.results_frame = ctk.CTkScrollableFrame(results_container)
        self.results_frame.grid(
            row=1, column=0, padx=12, pady=(0, 12), sticky="nsew"
        )

        for column in range(ROOMS_PER_ROW):
            self.results_frame.grid_columnconfigure(
                column,
                weight=1,
                uniform="room-column"
            )

        self.empty_results_label = ctk.CTkLabel(
            self.results_frame,
            text="Prikaz rasporeda po učinionicama",
            font=ctk.CTkFont(size=15)
        )
        self.empty_results_label.grid(
            row=0, column=0, columnspan=ROOMS_PER_ROW, padx=20, pady=60
        )


        # dataset
        def select_dataset(self):
            file_path = filedialog.askopenfilename(
                title="Odaberi dataset",
                initialdir=DATASETS_DIR,
                filetypes=[
                    ("TIM files", "*.tim"),
                    ("All files", "*.*")
                ]
            )

            if not file_path:
                return

            dataset = Path(file_path)

            match = re.fullmatch(r"dataset(\d+)", dataset.stem, flags=re.IGNORECASE)

            if not match:
                messagebox.showerror(
                    "Neispravan naziv dataseta",
                    "Trenutni C++ kod očekuje datoteke naziva dataset1.tim, "
                    "dataset2.tim, ..., dataset24.tim."
                )
                return

            dataset_number = int(match.group(1))

            if not 1 <= dataset_number <= 24:
                messagebox.showerror(
                    "Neispravan dataset",
                    "Broj dataseta mora biti između 1 i 24."
                )
                return

            self.selected_dataset = dataset
            self.selected_dataset_number = dataset_number

            self.dataset_label.configure(
                text=f"Odabrano: {dataset.name}"
            )
            self.status_label.configure(
                text=f"Spremno za pokretanje algoritma nad {dataset.name}."
            )

            self.clear_results(
                message="Odaberi Greedy, Local Search 1 ili 2 za prikaz rasporeda."
            )


        # pokretanje algoritama
        def start_greedy(self):
            if not self.ensure_dataset_selected():
                return
    
            if self.is_running:
                return
    
            self.set_running_state(True)
            self.status_label.configure(
                text=f"Greedy: build i pokretanje za dataset{self.selected_dataset_number}..."
            )
    
            threading.Thread(
                target=self.greedy_worker,
                daemon=True
            ).start()
    
        def greedy_worker(self):
            try:
                build_result = self.build_project(GREEDY_DIR)
    
                if build_result.returncode != 0:
                    raise RuntimeError(
                        self.format_process_error("Greedy build nije uspio", build_result)
                    )
    
                run_result = self.run_executable(
                    GREEDY_DIR,
                    self.selected_dataset_number
                )
    
                if run_result.returncode != 0:
                    raise RuntimeError(
                        self.format_process_error("Greedy nije uspješno izvršen", run_result)
                    )
    
                output_file = (
                    GREEDY_OUTPUT_DIR
                    / f"raspored_dataset{self.selected_dataset_number}_greedy.txt"
                )
    
                self.after(
                    0,
                    self.algorithm_finished,
                    "Greedy",
                    output_file
                )
    
            except Exception as error:
                self.after(0, self.algorithm_failed, "Greedy", str(error))        


if __name__ == "__main__":
    app = App()
    app.mainloop()