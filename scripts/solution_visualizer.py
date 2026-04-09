import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import matplotlib as mpl
from mpl_toolkits.axes_grid1 import make_axes_locatable

class SolutionVisualizer:
    @staticmethod
    def save_solution_plot(instance_file: Path, path: list, output_dir: Path, filename: str, title: str) -> Path:
        save_file = output_dir / filename
        SolutionVisualizer.plot_solution(instance_file, path, save_file, title)
        return save_file

    @staticmethod
    def plot_solution(instance_file: Path, path: list, save_file: Path, title: str):
        figure_width = 20 
        figure_height = 20

        min_node_size = 40     
        max_node_size = 400    
        
        alpha_wypełnienia = 0.7 
        alpha_obwódki = 1.0     
        cmap = plt.cm.Reds     
        
        path_color = 'black'
        path_width = 3.0        
        
        title_font_size = 28        
        colorbar_label_size = 22    
        colorbar_tick_size = 18     

        pos = {}
        profits_map = {}
        try:
            with open(instance_file, "r", encoding="utf-8") as f:
                node_id = 0
                for line in f:
                    line = line.strip()
                    if not line:
                        continue
                    parts = line.replace(';', ' ').replace(',', ' ').split()
                    if len(parts) >= 3:
                        x = float(parts[-3])
                        y = float(parts[-2])
                        profit = int(parts[-1])
                        
                        pos[node_id] = (x, y)
                        profits_map[node_id] = profit
                        node_id += 1
        except Exception as e:
            print(f"Błąd wczytywania instancji do wizualizacji: {e}")
            return
        
        all_node_ids = sorted(list(profits_map.keys()))
        all_profits = np.array(list(profits_map.values()))
        p_min = all_profits.min()
        p_max = all_profits.max()
        p_range = p_max - p_min if p_max > p_min else 1.0

        a_coords = []
        a_sizes = []
        a_face_colors = []
        a_edge_colors = []

        for node in all_node_ids:
            profit = profits_map[node]
            norm_p = (profit - p_min) / p_range if p_range > 0 else 0.0
            
            a_coords.append(pos[node])
            size = min_node_size + norm_p * (max_node_size - min_node_size)
            a_sizes.append(size)
            
            base_mapped_color = cmap(0.3 + 0.7 * norm_p) 
            a_face_colors.append((base_mapped_color[0], base_mapped_color[1], base_mapped_color[2], alpha_wypełnienia))
            a_edge_colors.append((base_mapped_color[0], base_mapped_color[1], base_mapped_color[2], alpha_obwódki))

        a_coords = np.array(a_coords)
        
        fig, ax = plt.subplots(figsize=(figure_width, figure_height))
        ax.set_aspect('equal')
        

        start_node_id = path[0]
        start_size = max_node_size * 4.0
        start_coords = pos[start_node_id]
        
        ax.scatter(start_coords[0], start_coords[1], 
                   s=start_size, marker='*',
                   facecolors='black', edgecolors='black', 
                   linewidths=1.0, alpha=1.0, 
                   zorder=9)
                   
        ax.scatter(a_coords[:, 0], a_coords[:, 1], 
                   s=a_sizes, facecolors=a_face_colors, edgecolors='none', zorder=10)

        path_coords = np.array([pos[node] for node in path])
        ax.plot(path_coords[:, 0], path_coords[:, 1], 
                color=path_color, linewidth=path_width, alpha=0.9, zorder=11)

        ax.scatter(a_coords[:, 0], a_coords[:, 1], 
                   s=a_sizes, facecolors='none', edgecolors=a_edge_colors, linewidths=1.5, zorder=12)
        
        plt.title(title, fontsize=title_font_size, pad=20)
        plt.axis('off') 
        
        norm_cbar = mpl.colors.Normalize(vmin=p_min, vmax=p_max)
        mappable_colorbar = mpl.cm.ScalarMappable(norm=norm_cbar, cmap=cmap)
        mappable_colorbar.set_array([]) 

        divider = make_axes_locatable(ax)
        cax = divider.append_axes("left", size="4%", pad=0.15)
        
        cbar = fig.colorbar(mappable_colorbar, cax=cax, orientation='vertical')
        cax.yaxis.set_ticks_position('left')
        cax.yaxis.set_label_position('left')
        
        cbar.set_label('Zysk wierzchołka', fontsize=colorbar_label_size)
        cax.tick_params(labelsize=colorbar_tick_size)
        cbar.set_alpha(1.0) 

        fig.tight_layout()
        
        save_file.parent.mkdir(parents=True, exist_ok=True)
        fig.savefig(save_file, dpi=300, bbox_inches='tight')
        plt.close(fig)