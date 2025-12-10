/**
 * Drawer Resize Functionality
 * Enables horizontal resizing of the navigation drawer with localStorage persistence
 * Desktop-only version
 */
(function() {
    'use strict';

    const STORAGE_KEY = 'opptheme-drawer-width';
    const MIN_WIDTH = 150;
    const MAX_WIDTH = 500;
    const DEFAULT_WIDTH = 240;

    let drawer = null;
    let handle = null;
    let isResizing = false;
    let startX = 0;
    let startWidth = 0;

    /**
     * Initialize the drawer resize functionality
     */
    function init() {
        drawer = document.querySelector('.mdl-layout__drawer');
        if (!drawer) return;

        createResizeHandle();
        loadSavedWidth();
        attachEventListeners();
    }

    /**
     * Create and insert the resize handle element
     */
    function createResizeHandle() {
        handle = document.createElement('div');
        handle.className = 'drawer-resize-handle';
        handle.setAttribute('title', 'Drag to resize');
        // Append to body since it uses fixed positioning
        document.body.appendChild(handle);
    }

    /**
     * Load saved width from localStorage and apply it
     */
    function loadSavedWidth() {
        try {
            const savedWidth = localStorage.getItem(STORAGE_KEY);
            if (savedWidth) {
                const width = parseInt(savedWidth, 10);
                if (width >= MIN_WIDTH && width <= MAX_WIDTH) {
                    setDrawerWidth(width);
                }
            }
        } catch (e) {
            console.warn('Could not load drawer width from localStorage:', e);
        }
    }

    /**
     * Save the current width to localStorage
     */
    function saveWidth(width) {
        try {
            localStorage.setItem(STORAGE_KEY, width.toString());
        } catch (e) {
            console.warn('Could not save drawer width to localStorage:', e);
        }
    }

    /**
     * Set the drawer width via CSS custom property
     */
    function setDrawerWidth(width) {
        document.documentElement.style.setProperty('--drawer-width', width + 'px');
    }

    /**
     * Get the current drawer width
     */
    function getDrawerWidth() {
        return drawer.getBoundingClientRect().width;
    }

    /**
     * Attach event listeners for resize functionality (desktop only)
     */
    function attachEventListeners() {
        handle.addEventListener('mousedown', onMouseDown);
        document.addEventListener('mousemove', onMouseMove);
        document.addEventListener('mouseup', onMouseUp);
    }

    /**
     * Handle mouse down on resize handle
     */
    function onMouseDown(e) {
        e.preventDefault();
        isResizing = true;
        startX = e.clientX;
        startWidth = getDrawerWidth();
        document.body.classList.add('drawer-resizing');
        handle.classList.add('dragging');
    }

    /**
     * Handle mouse move during resize
     */
    function onMouseMove(e) {
        if (!isResizing) return;
        e.preventDefault();

        const delta = e.clientX - startX;
        let newWidth = startWidth + delta;

        // Enforce min/max constraints
        newWidth = Math.max(MIN_WIDTH, Math.min(MAX_WIDTH, newWidth));

        setDrawerWidth(newWidth);
    }

    /**
     * Handle mouse up to end resize
     */
    function onMouseUp(e) {
        if (!isResizing) return;

        isResizing = false;
        document.body.classList.remove('drawer-resizing');
        handle.classList.remove('dragging');

        const finalWidth = getDrawerWidth();
        saveWidth(Math.round(finalWidth));
    }

    // Initialize when DOM is ready
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();

