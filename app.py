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
TABU_SEARCH_DIR = BASE_DIR / "local_search"
LOCAL_SEARCH_DIR = BASE_DIR / "local_search"

GREEDY_OUTPUT_DIR = BASE_DIR / "greedy_outputs"
TABU_SEARCH_OUTPUT_DIR = BASE_DIR / "tabu_outputs"
LOCAL_SEARCH_1_OUTPUT_DIR = BASE_DIR / "ls_outputs/ls1_outputs/"
LOCAL_SEARCH_2_OUTPUT_DIR = BASE_DIR / "ls_outputs/ls2_outputs/"
CHECK_EXECUTABLE = BASE_DIR / "check.exe"

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
        self.tabu_ready_for_dataset = None
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

    def scroll_results(self, event):
        amount = int(-2 * (event.delta / 10))

        self.results_frame._parent_canvas.yview_scroll(amount, "units")
        return "break"

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

        self.clean_button = ctk.CTkButton(
            section,
            text="Očisti datoteke",
            width=180,
            height=40,
            command=self.start_clean
        )
        self.clean_button.grid(row=1, column=0, padx=(18, 12), pady=(0, 16))

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
            text="Najbolji poboljšavajući susjed",
            height=44,
            command=self.start_local_search_1
        )
        self.local_search_1_button.grid(
            row=0, column=1, padx=8, pady=(16, 8), sticky="ew"
        )

        self.local_search_2_button = ctk.CTkButton(
            section,
            text="Prvi poboljšavajući susjed",
            height=44,
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
        results_container.grid_rowconfigure(3, weight=1)

        self.results_title = ctk.CTkLabel(
            results_container,
            text="Raspored",
            anchor="w",
            font=ctk.CTkFont(size=20, weight="bold")
        )
        self.results_title.grid(
            row=0, column=0, padx=18, pady=(16, 8), sticky="ew"
        )

        self.unplaced_label = ctk.CTkLabel(
            results_container,
            text="",
            anchor="w",
            justify="left",
            font=ctk.CTkFont(size=14)
        )

        self.unplaced_label.grid(
            row=2, column=0, padx=18, pady=(0, 8), sticky="ew"
        )

        self.check_label = ctk.CTkLabel(
            results_container,
            text="",
            anchor="w",
            justify="left",
            font=ctk.CTkFont(size=14)
        )
        self.check_label.grid(
            row=1, column=0, padx=18, pady=(0, 8), sticky="ew"
        )

        self.results_frame = ctk.CTkScrollableFrame(results_container)
        self.results_frame.grid(
            row=3, column=0, padx=12, pady=(0, 12), sticky="nsew"
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


    # tabu search 
    def prepare_tabu_baseline(self):
        dataset_number = self.selected_dataset_number

        tabu_output = (
            TABU_SEARCH_OUTPUT_DIR /
            f"ts_output{dataset_number}.sln"
        )

        if (
            self.tabu_ready_for_dataset == dataset_number
            and tabu_output.exists()
        ):
            return tabu_output

        # greedy rjesenje
        build_result = self.build_project(GREEDY_DIR)

        if build_result.returncode != 0:
            raise RuntimeError(
                self.format_process_error(
                    "Greedy build nije uspio",
                    build_result
                )
            )

        greedy_run = self.run_executable(
            GREEDY_DIR,
            dataset_number
        )

        if greedy_run.returncode != 0:
            raise RuntimeError(
                self.format_process_error(
                    "Greedy nije uspjesno izvrsen",
                    greedy_run
                )
            )

        # tabu search

        self.after(
            0,
            self.status_label.configure,
            {
                "text": (
                    f"Priprema zajednickog Tabu Search rjesenja za "
                    f"dataset{dataset_number}..."
                )
            }
        )

        tabu_build = self.build_project(TABU_SEARCH_DIR)

        if tabu_build.returncode != 0:
            raise RuntimeError(
                self.format_process_error(
                    "Tabu Search build nije uspio",
                    tabu_build
                )
            )


        tabu_run = self.run_executable(
            TABU_SEARCH_DIR,
            dataset_number,
            -1
        )

        if tabu_run.returncode != 0:
            raise RuntimeError(
                self.format_process_error(
                    "Tabu Search nije uspjesno izvrsen",
                    tabu_run
                )
            )

        if not tabu_output.exists():
            raise FileNotFoundError(
                f"Tabu Search nije proizveo:\n{tabu_output}"
            )

        self.tabu_ready_for_dataset = dataset_number

        return tabu_output

    def start_local_search_1(self):
        if not self.ensure_dataset_selected():
            return

        if self.is_running:
            return

        self.set_running_state(True)
        self.status_label.configure(
            text=(
                f"Local Search 1: priprema Greedy rasporeda za "
                f"dataset{self.selected_dataset_number}..."
            )
        )

        threading.Thread(
            target=self.local_search_1_worker,
            daemon=True
        ).start()

    def local_search_1_worker(self):
        try:
            self.after(
                0,
                self.status_label.configure,
                {
                    "text": (
                        f"Priprema Tabu Search rjesenja "
                        f"dataset{self.selected_dataset_number}..."
                    )
                }
            )

            tabu_output = self.prepare_tabu_baseline()

            self.after(
                0,
                self.status_label.configure,
                {
                    "text": (
                        f"Local Search 1: build i optimizacija "
                        f"dataset{self.selected_dataset_number}..."
                    )
                }
            )

            ls1_build = self.build_project(LOCAL_SEARCH_DIR)

            if ls1_build.returncode != 0:
                raise RuntimeError(
                    self.format_process_error(
                        "Local Search 1 build nije uspio",
                        ls1_build
                    )
                )

            ls1_run = self.run_executable(
                LOCAL_SEARCH_DIR,
                self.selected_dataset_number,
                1
            )

            if ls1_run.returncode != 0:
                raise RuntimeError(
                    self.format_process_error(
                        "Local Search 1 nije uspješno izvršen",
                        ls1_run
                    )
                )

            output_file = (
                LOCAL_SEARCH_1_OUTPUT_DIR
                / f"ls1_output{self.selected_dataset_number}.sln"
            )

            self.after(
                0,
                self.algorithm_finished,
                "Local Search 1",
                output_file
            )

        except Exception as error:
            self.after(0, self.algorithm_failed, "Local Search 1", str(error))

    def start_local_search_2(self):
        if not self.ensure_dataset_selected():
            return

        if self.is_running:
            return

        self.set_running_state(True)
        self.status_label.configure(
            text=(
                f"Local Search 2: priprema Greedy rasporeda za "
                f"dataset{self.selected_dataset_number}..."
            )
        )

        threading.Thread(
            target=self.local_search_2_worker,
            daemon=True
        ).start()

    def start_clean(self):
        if self.is_running:
            return

        self.set_running_state(True)
        self.status_label.configure(text="Brisanje generiranih datoteka...")

        threading.Thread(
            target=self.clean_worker,
            daemon=True
        ).start()

    def clean_worker(self):
        try:
            removed_files = self.clean_generated_files()
            self.after(
                0,
                self.clean_finished,
                removed_files
            )
        except Exception as error:
            self.after(0, self.algorithm_failed, "Brisanje", str(error))

    def clean_finished(self, removed_files):
        self.set_running_state(False)
        self.clear_results(
            message="Generirane datoteke su očišćene."
        )
        self.status_label.configure(
            text=(
                f"Brisanje završeno. Uklonjeno datoteka: {removed_files}."
            )
        )

    def local_search_2_worker(self):
        try:
            self.after(
                0,
                self.status_label.configure,
                {
                    "text": (
                        f"Priprema Tabu Search rjesenja "
                        f"dataset{self.selected_dataset_number}..."
                    )
                }
            )
            tabu_output = self.prepare_tabu_baseline()
    
            ls2_build = self.build_project(LOCAL_SEARCH_DIR)
            if ls2_build.returncode != 0:
                raise RuntimeError(
                    self.format_process_error(
                        "Local Search 2 build nije uspio",
                        ls2_build
                    )
                )
    
            ls2_run = self.run_executable(
                LOCAL_SEARCH_DIR,
                self.selected_dataset_number,
                2
            )
            if ls2_run.returncode != 0:
                raise RuntimeError(
                    self.format_process_error(
                        "Local Search 2 nije uspješno izvršen",
                        ls2_run
                    )
                )
    
            output_file = (
                LOCAL_SEARCH_2_OUTPUT_DIR
                / f"ls2_output{self.selected_dataset_number}.sln"
            )
    
            self.after(
                0,
                self.algorithm_finished,
                "Local Search 2",
                output_file
            )
    
        except Exception as error:
            self.after(0, self.algorithm_failed, "Local Search 2", str(error))

    
    # Build / subprocess pomocne metode
    @staticmethod
    def check_schedule(dataset_file, output_file):
        if not CHECK_EXECUTABLE.exists():
            raise FileNotFoundError(
                f"Checker nije pronađen: {CHECK_EXECUTABLE}"
            )

        return subprocess.run(
            [
                str(CHECK_EXECUTABLE),
                str(dataset_file),
                str(output_file)
            ],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace"
        )

    def build_project(self, project_dir):
        if not project_dir.exists():
            raise FileNotFoundError(f"Folder ne postoji: {project_dir}")

        make_command = self.find_make_command()

        return subprocess.run(
            [make_command],
            cwd=project_dir,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace"
        )

    def clean_generated_files(self):
        make_command = self.find_make_command()
        clean_directories = {GREEDY_DIR, LOCAL_SEARCH_DIR}

        for project_dir in clean_directories:
            clean_result = subprocess.run(
                [make_command, "clean"],
                cwd=project_dir,
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace"
            )

            if clean_result.returncode != 0:
                raise RuntimeError(
                    self.format_process_error(
                        f"Čišćenje foldera {project_dir.name} nije uspjelo",
                        clean_result
                    )
                )

        removed_files = 0
        for file_path in BASE_DIR.rglob("*"):
            if ".git" in file_path.parts:
                continue

            if file_path.is_file() and file_path.suffix.lower() in {".sln", ".txt"}:
                file_path.unlink()
                removed_files += 1

        return removed_files

    def run_executable(self, project_dir, dataset_number, method_number=None):
        executable = self.find_executable(project_dir)

        input = f"{dataset_number}\n"

        if method_number is not None:
            input += f"{method_number}\n"
        
        return subprocess.run(
            [str(executable)],
            cwd=project_dir,
            input=input,
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace"
        )

    @staticmethod
    def find_make_command():
        #kako bi radilo za obojicu jer jedan ima make, a drugi mingw32-make
        for command in ("make", "mingw32-make"):
            if shutil.which(command):
                return command

        raise FileNotFoundError(
            "Nije pronađen 'make' niti 'mingw32-make'. "
        )

    @staticmethod
    def find_executable(project_dir):
        candidates = [
            project_dir / "main.exe",
            project_dir / "main"
        ]

        for candidate in candidates:
            if candidate.exists():
                return candidate

        raise FileNotFoundError(
            f"Nakon builda nije pronađen main.exe niti main u folderu: "
            f"{project_dir}"
        )

    @staticmethod
    def format_process_error(title, result):
        details = (result.stderr or result.stdout or "").strip()

        if not details:
            details = f"Proces je završio s kodom {result.returncode}."

        return f"{title}.\n\n{details}"


    # rezultati i njihov prikaz
    def algorithm_finished(self, algorithm_name, output_file):
        try:
            if not output_file.exists():
                raise FileNotFoundError(
                    f"Algoritam je završio, ali izlazna datoteka ne postoji:\n"
                    f"{output_file}"
                )

            check_result = self.check_schedule(
                self.selected_dataset,
                output_file
            )

            room_capacities = self.read_room_capacities(self.selected_dataset)
            schedule, unplaced = self.read_schedule(output_file)

            self.display_schedule(
                algorithm_name,
                schedule,
                room_capacities,
                unplaced
            )

            if check_result.returncode > 0:
                self.check_label.configure(
                    text=(
                        f"Trošak povrede mekih uvjeta: {check_result.returncode}"
                    )
                )
            elif check_result.returncode < 0:
                self.check_label.configure(
                    text=(
                        f"Trošak povrede tvrdih uvjeta: {-check_result.returncode}"
                    )
                )
            else:
                self.check_label.configure(
                    text="Raspored je dopustiv bez troška."
                )

            self.status_label.configure(
                text=(
                    f"{algorithm_name} uspješno završen. "
                    f"Prikazan je rezultat za {self.selected_dataset.name}."
                )
            )

        except Exception as error:
            messagebox.showerror(
                "Greška pri prikazu rezultata",
                str(error)
            )
            self.status_label.configure(
                text=f"{algorithm_name} je završen, ali rezultat nije moguće prikazati."
            )

        finally:
            self.set_running_state(False)

    def algorithm_failed(self, algorithm_name, error):
        self.set_running_state(False)
        self.status_label.configure(
            text=f"{algorithm_name}: dogodila se greška."
        )

        messagebox.showerror(
            f"Greška - {algorithm_name}",
            error
        )

    @staticmethod
    def read_schedule(output_file):
        """
        C++ zapis:
            redak 0 -> event 0 -> "timeslot room"
            redak 1 -> event 1 -> "timeslot room"
            ...

        Povratna vrijednost:
            dict[room][timeslot] = event
        """
        schedule_by_room = {}
        unplaced_events = []

        with output_file.open(
            "r",
            encoding="utf-8",
            errors="replace"
        ) as file:
            for event_number, raw_line in enumerate(file):
                line = raw_line.strip()

                if not line:
                    continue

                parts = line.split()

                if len(parts) != 2:
                    raise ValueError(
                        f"Neispravan redak u {output_file.name}: {line}"
                    )

                timeslot, room = map(int, parts)

                # -1 -1 = neraspoređeni event
                if timeslot == -1 and room == -1:
                    unplaced_events.append(int(event_number))
                    continue

                if not 0 <= timeslot < NUMBER_OF_TIMESLOTS:
                    raise ValueError(
                        f"Neispravan timeslot {timeslot} za event {event_number}."
                    )

                if room < 0:
                    raise ValueError(
                        f"Neispravna učionica {room} za event {event_number}."
                    )

                schedule_by_room.setdefault(room, {})[timeslot] = event_number

        return schedule_by_room, unplaced_events

    @staticmethod
    def read_room_capacities(dataset_file):
        """
        .tim format počinje s:
            E R F S

        Nakon toga slijedi R kapaciteta učionica.
        """
        with dataset_file.open(
            "r",
            encoding="utf-8",
            errors="replace"
        ) as file:
            values = file.read().split()

        if len(values) < 4:
            raise ValueError("Dataset nema ispravno zaglavlje E R F S.")

        number_of_rooms = int(values[1])

        first_capacity_index = 4
        last_capacity_index = first_capacity_index + number_of_rooms

        if len(values) < last_capacity_index:
            raise ValueError("Dataset nema sve kapacitete učionica.")

        return [
            int(value)
            for value in values[first_capacity_index:last_capacity_index]
        ]

    def display_schedule(self, algorithm_name, schedule, room_capacities, unplaced_events):
        self.clear_results()

        self.results_title.configure(
            text=(
                f"{algorithm_name} — {self.selected_dataset.name} "
                f"({len(room_capacities)} učionica)"
            )
        )

        if len(unplaced_events) > 0:
                self.unplaced_label.configure(
                    text=(
                        f"Neraspoređeni predmeti ({len(unplaced_events)}): "
                        + ", ".join(
                            f"E{event}"
                            for event in unplaced_events
                        )
                    )
                )

        for room_index in range(len(room_capacities)):
            row = room_index // ROOMS_PER_ROW
            column = room_index % ROOMS_PER_ROW

            room_card = self.create_room_card(
                room_index,
                room_capacities[room_index],
                schedule.get(room_index, {})
            )

            room_card.grid(
                row=row,
                column=column,
                padx=8,
                pady=8,
                sticky="nsew"
            )

    def create_room_card(self, room_number, capacity, room_schedule):
        card = ctk.CTkFrame(
            self.results_frame,
            border_width=1
        )

        card.grid_columnconfigure(0, weight=1)

        title = ctk.CTkLabel(
            card,
            text=f"Učionica {room_number + 1}",
            font=ctk.CTkFont(size=16, weight="bold")
        )
        title.grid(row=0, column=0, padx=10, pady=(12, 2), sticky="ew")

        capacity_label = ctk.CTkLabel(
            card,
            text=f"Kapacitet: {capacity}",
            font=ctk.CTkFont(size=12)
        )
        capacity_label.grid(row=1, column=0, padx=10, pady=(0, 8), sticky="ew")

        # tekstualna mini-tablica je kompaktnija i omogucava da 4 ucionice
        # stanu u jedan red cak i na manjim ekranima.
        lines = []

        header = "Termin | " + " | ".join(f"{day:^5}" for day in DAYS)
        lines.append(header)
        lines.append("-" * len(header))

        for slot_in_day in range(SLOTS_PER_DAY):
            cells = []

            for day_index in range(len(DAYS)):
                timeslot = day_index * SLOTS_PER_DAY + slot_in_day
                event = room_schedule.get(timeslot)

                if event is None:
                    cells.append("  -  ")
                else:
                    cells.append(f" E{event:<3}")

            lines.append(
                f"{slot_in_day + 1:>6} | " + " | ".join(cells)
            )

        schedule_text = ctk.CTkTextbox(
            card,
            height=245,
            font=ctk.CTkFont(family="Consolas", size=11),
            wrap="none"
        )
        schedule_text.grid(
            row=2, column=0, padx=10, pady=(0, 12), sticky="nsew"
        )
        schedule_text.insert("1.0", "\n".join(lines))
        schedule_text.configure(state="disabled")

        schedule_text.bind("<MouseWheel>", self.scroll_results)

        return card

    def clear_results(self, message=None):
        self.check_label.configure(text="")
        self.unplaced_label.configure(text="")

        for widget in self.results_frame.winfo_children():
            widget.destroy()

        if message:
            label = ctk.CTkLabel(
                self.results_frame,
                text=message,
                font=ctk.CTkFont(size=15)
            )
            label.grid(
                row=0,
                column=0,
                columnspan=ROOMS_PER_ROW,
                padx=20,
                pady=60
            )


    # stanje GUI-ja
    def ensure_dataset_selected(self):
        if self.selected_dataset is None:
            messagebox.showwarning(
                "Dataset nije odabran",
                "Prvo odaberi dataset."
            )
            return False

        return True

    def set_running_state(self, running):
        self.is_running = running

        state = "disabled" if running else "normal"

        self.dataset_button.configure(state=state)
        self.greedy_button.configure(state=state)
        self.local_search_1_button.configure(state=state)
        self.local_search_2_button.configure(state=state)
        self.clean_button.configure(state=state)


if __name__ == "__main__":
    app = App()
    app.mainloop()