import "vite/modulepreload-polyfill";

// Htmx setup
import htmx from "htmx.org";

// Expose htmx on window
// @ts-ignore-next-line
window.htmx = htmx;
