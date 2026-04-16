# 🔥 Fluid Simulation in C++ (Stable Fluids - Jos Stam Method)

This project implements a **2D fluid simulation** using the **Stable Fluids algorithm** (diffusion, advection, projection) with real-time rendering using OpenGL via GLFW.

---

## 📁 Project Structure

```
.
├── main_fixed.cpp
├── fluid_algo.cpp
├── fluid_struct.h
```

---

## 📌 File Descriptions

- **main_fixed.cpp** → Rendering + simulation loop  
- **fluid_algo.cpp** → Fluid physics (diffuse, advect, project)  
- **fluid_struct.h** → Data structures and constants  

---

## ⚙️ Requirements

- g++ (MinGW recommended)
- GLFW library
- OpenGL (comes with Windows)

---

## 🛠️ Compile (Windows)

```
g++ main_fixed.cpp fluid_algo.cpp ^
-IC:\libraries\glfw\include ^
-LC:\libraries\glfw\lib-mingw-w64 ^
-lglfw3 -lopengl32 -lgdi32 ^
-o fluid.exe
```

---

## ▶️ Run

```
fluid.exe
```

---

## ⚠️ Common Issues

- Permission denied → close running .exe  
- Missing DLL → copy glfw3.dll to project folder  
- Include error → check GLFW paths  

---

## 🚀 Git Setup

```
git init
git add .
git commit -m "Initial commit"
git remote add origin YOUR_REPO_URL
git push -u origin main
```

---

## 🧠 Algorithm

- Diffusion → viscosity  
- Advection → fluid movement  
- Projection → incompressibility  

---

## ✨ Future Improvements

- Mouse interaction  
- GPU acceleration  
- Better visualization  

---

## 👨‍🔬 Author

Akhilesh Ray  
Research Scholar (Astrophysical Fluid Dynamics)
