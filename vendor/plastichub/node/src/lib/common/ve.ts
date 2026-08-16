/**
 * WIDGET_REFERENCE_MODE enumerates possible modes to resolve a string expression
 * into instances. There are a few CI based widgets subclassed from xide/widgets/Referenced.
 * The reference structure consist out of this mode and that expression.
 *
 * @constant {Array.<module:xide/types~WidgetReferenceMode>}
 *     module:xide/types~WIDGET_REFERENCE_MODE
 */
export const WIDGET_REFERENCE_MODE = {
    BY_ID: 'byid',
    BY_CLASS: 'byclass',
    BY_CSS: 'bycss',
    BY_EXPRESSION: 'expression'
};
