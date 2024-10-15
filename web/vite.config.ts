import { defineConfig } from "vite";
import path from "path";
import { readdirSync } from "fs";

function underDirectory(dirPath: string, extension: string): string[] {
    return readdirSync(path.resolve(__dirname, dirPath), { withFileTypes: true })
        .filter((item) => !item.isDirectory() && item.name.endsWith(extension))
        .map((item) => item.parentPath + "/" + item.name);
}

export default defineConfig({
    root: "front",
    plugins: [],
    build: {
        emptyOutDir: true,
        outDir: path.resolve(__dirname, "./front/dist/"),

        // make a manifest and source maps.
        manifest: "manifest.json",
        sourcemap: true,
        minify: false,
        terserOptions: { compress: false, mangle: false },

        rollupOptions: {
            // All .ts files under ./front become entry points
            input: underDirectory("./front", ".ts")
        }
    },

    resolve: {
        alias: [
            {
                find: "front",
                replacement: path.resolve(__dirname, "./front")
            }
        ]
    },

    esbuild: {},
    optimizeDeps: {
        include: []
    }
});
